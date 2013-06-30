To compile gm_luaerror2 for Mac or Linux copy the libraries tier0 and tier1 from SourceSDK/lib/mac or SourceSDK/lib/linux, respectively, to the corresponding project folders. This is needed because of the way they're linked on these OSes. This is confirmed, at least, on Linux.

Mac was not tested at all (sorry but I'm cheap and lazy).

If stuff starts erroring or fails to work, be sure to check the correct line endings (\n and such) are present in the files for each OS.

Be sure to set GARRYSMOD_INCLUDES_PATH and SOURCE_SDK_PATH to the Garry's Mod module base and the SourceSDK path, respectively!

The required Garry's Mod headers to build modules are already included as externals. Thank me later. You might also find some useful stuff there.