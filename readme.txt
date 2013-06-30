To compile gm_luaerror2 for Mac or Linux copy the libraries tier0 and tier1 from SourceSDK/lib/mac or SourceSDK/lib/linux, respectively, to the corresponding project folders. This is needed because of the way they're linked on these OSes. This is confirmed, at least, on Linux.

Mac was not tested at all (sorry but I'm cheap and lazy).

If stuff starts erroring or fails to work, be sure to check the correct line endings (\n and such) are present in the files for each OS.

Be sure to set GARRYSMOD_INCLUDES_PATH and SOURCE_SDK_PATH to the Garry's Mod module base and the SourceSDK path, respectively!

You will need some of the interfaces provided by my version of the headers repository for Garry's Mod here: https://bitbucket.org/tuestu1/gmod-module-base