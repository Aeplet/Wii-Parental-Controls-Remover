#include <ogc/ipc.h>
#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ogc/isfs.h>

// SYSCONF reading code taken from libogc
static int __conf_inited = 0;
static s32 sysconf_size = 0x4000;
static u8 __conf_buffer[0x4000] ATTRIBUTE_ALIGN(32);
static const char __conf_file[] ATTRIBUTE_ALIGN(32) = "/shared2/sys/SYSCONF";

const u8 parental_enabled_pattern[] = { 0x49, 0x50, 0x4C, 0x2E, 0x50, 0x43, 0x49 }; // "IPL.PCI"
const size_t pattern_len = sizeof(parental_enabled_pattern);

s32 SYSCONF_Init(void)
{
	int fd;
	int ret;
	
	if(__conf_inited) return 0;
	
	fd = IOS_Open(__conf_file,1);
	if(fd < 0) return fd;
	
	memset(__conf_buffer, 0, sysconf_size);
	
	ret = IOS_Read(fd, __conf_buffer, sysconf_size);
	IOS_Close(fd);
	if(ret != sysconf_size) return CONF_EBADFILE;
			
	if(memcmp(__conf_buffer, "SCv0", 4)) return CONF_EBADFILE;
		
	__conf_inited = 1;
	return 0;
}

bool parental_enabled(void)
{
    // Probably a better way to do this
    for (size_t i = 0; i < sizeof(__conf_buffer) - pattern_len; ++i) {
        if (memcmp(&__conf_buffer[i], parental_enabled_pattern, pattern_len) == 0) {
            u8 next_byte = __conf_buffer[i + pattern_len];
            return next_byte == 0x80 ? true : false;
        }
    }
    return false;
}

bool set_parental(s32 val)
{
    u8 new_sysconf[sizeof(__conf_buffer)];
    memcpy(new_sysconf, __conf_buffer, sizeof(__conf_buffer));

    for (size_t i = 0; i < sizeof(new_sysconf) - pattern_len; ++i) {
        if (memcmp(&new_sysconf[i], parental_enabled_pattern, pattern_len) == 0) {
            new_sysconf[i + pattern_len] = val;
        }
    }

    // Be nice with NAND Writes. Please.
    s32 fd = IOS_Open(__conf_file, 2);
    if (fd < 0) return false;

    IOS_Write(fd, new_sysconf, sysconf_size);
    // Can't forget the close
    IOS_Close(fd);
    return true;
}