#include "dev_presence.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

static SDL_bool is_char_device_present(dev_t devnum);

static int is_hidraw(const struct dirent *dir);

static int is_evdev(const struct dirent *dir);

static int is_jsdev(const struct dirent *dir);

Uint32 SDL_webOSGetDevicePresenceFlags(SDL_webOSDevicePresenceCheck check)
{
    Uint32 flags = 0;
    struct dirent **dev_list;
    int dev_dir_fd;
    int dev_count;
    const char *base_dir;
    int (*selector)(const struct dirent *);
    int prefix_len;

    switch (check) {

    case SDL_WEBOS_DEVICE_PRESENCE_CHECK_HIDRAW:
        base_dir = "/dev";
        selector = is_hidraw;
        prefix_len = 6;
        break;
    case SDL_WEBOS_DEVICE_PRESENCE_CHECK_EVDEV:
        base_dir = "/dev/input";
        selector = is_evdev;
        prefix_len = 5;
        break;
    case SDL_WEBOS_DEVICE_PRESENCE_CHECK_JS:
        base_dir = "/dev/input";
        selector = is_jsdev;
        prefix_len = 2;
        break;
    default:
        return 0;
    }

    dev_dir_fd = open(base_dir, O_RDONLY | O_DIRECTORY);

    if (dev_dir_fd < 0) {
        return 0;
    }

    dev_count = scandir(base_dir, &dev_list, selector, alphasort);

    if (dev_count < 0) {
        close(dev_dir_fd);
        return 0;
    }

    for (int dev_idx = 0; dev_idx < dev_count; dev_idx++) {
        struct stat dev_st;
        int ret;
        int dev_num;
        char *endptr = NULL;
        ret = fstatat(dev_dir_fd, dev_list[dev_idx]->d_name, &dev_st, 0);
        if (ret != 0 || !S_ISCHR(dev_st.st_mode)) {
            free(dev_list[dev_idx]); /* SHOULD NOT be freed with SDL_free() */
            continue;
        }
        dev_num = strtol(dev_list[dev_idx]->d_name + prefix_len, &endptr, 10);
        if (endptr == NULL || *endptr != '\0') {
            free(dev_list[dev_idx]); /* SHOULD NOT be freed with SDL_free() */
            continue;
        }
        if (is_char_device_present(dev_st.st_rdev)) {
            flags |= 1 << dev_num;
        }
        free(dev_list[dev_idx]); /* SHOULD NOT be freed with SDL_free() */
    }
    free(dev_list); /* SHOULD NOT be freed with SDL_free() */

    close(dev_dir_fd);

    return flags;
}

extern SDL_bool SDL_webOSIsDeviceIndexPresent(Uint32 flags, int index)
{
    return (flags & (1 << index)) != 0;
}

static SDL_bool is_char_device_present(dev_t devnum)
{
    char path[256];
    struct stat st;
    snprintf(path, 256, "/sys/dev/char/%u:%u", major(devnum), minor(devnum));
    return stat(path, &st) == 0;
}

int is_hidraw(const struct dirent *dir)
{
    return dir->d_type == DT_CHR && strncmp(dir->d_name, "hidraw", 6) == 0;
}

int is_evdev(const struct dirent *dir)
{
    return dir->d_type == DT_CHR && strncmp(dir->d_name, "event", 5) == 0;
}

int is_jsdev(const struct dirent *dir)
{
    return dir->d_type == DT_CHR && strncmp(dir->d_name, "js", 2) == 0;
}
