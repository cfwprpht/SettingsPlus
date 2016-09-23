# Settings+ #

This is a proof of concept for a system application manipulation via a replicated and patched module.


## Modifications ##
As this is only a POC release, not many things can be done yet.

You can currently modify:
- version.txt        : You can input a custom UTF8 string with max 28 characters.
- mac.txt            : You can input a custom UTF8 string with max 18 characters.
- console_info.xml   : You can change the order and/or remove the entries
- system_settings.xml: You can change the order and/or remove the entries

Spoofed/modded files go to ux0:app/FLOW10015.

If you want to replace the original Settings application, you can edit the path in app.db to 'ux0:app/FLOW10015/eboot.bin'.


## To developers ##
A list of nids is included at 'nids.txt' where they can all be redirected as shown in the POC plugin.

Included in the src is only the patcher plugin 'flow.suprx', not the tool to generate the eboot.bin.
