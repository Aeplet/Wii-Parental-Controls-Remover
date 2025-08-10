#include <gccore.h>
#include <stdio.h>

extern s32 CONF_Get(const char *name, void *buffer, u32 length);
extern s32 CONF_Set(const char *name, const void *buffer, u32 length);

const s32 parental_enabled_value = 0x80; // 0x00 = off
const s32 IPL_PC_SIZE = 0x4A;

s8 get_ipl_pc_section(u8 *buf)
{
	int res = CONF_Get("IPL.PC", buf, IPL_PC_SIZE);
    if (res == IPL_PC_SIZE) return 0;
    return -1;
}

bool parental_enabled(void)
{
	u8 buf[0x4A];
    get_ipl_pc_section(buf);

    if (buf[0])
    {
        int val = buf[0];
        return val == parental_enabled_value ? true : false;
    }
    // Should we be returning false as a fallback? Shouldn't we do something else?
    return false;
}

bool set_parental(s32 val)
{
    u8 buf[IPL_PC_SIZE];
    get_ipl_pc_section(buf);
    buf[0] = val;

    s32 ret = CONF_Set("IPL.PC", buf, IPL_PC_SIZE);
    if (ret == IPL_PC_SIZE)
    {
        printf("Successfully disabled parental controls!\n");
        return true;
    }
    // uhhhh wtf happened lol do they have a corrupt SYSCONF or? wtf
    printf("Failed to disable parental controls? (CONF_Set returned %d)", ret);
    return false;
}