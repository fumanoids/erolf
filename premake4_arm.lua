TOOLCHAIN_PREFIX				= "/home/lutz/gcc-arm-none-eabi-4.6/bin"

GCC_ARM_CC_PATH  = TOOLCHAIN_PREFIX .. "/arm-none-eabi-gcc"
GCC_ARM_CPP_PATH = TOOLCHAIN_PREFIX .. "/arm-none-eabi-g++"
GCC_ARM_AR_PATH  = TOOLCHAIN_PREFIX .. "/arm-none-eabi-ar"
GCC_ARM_OBJCPY 	 = TOOLCHAIN_PREFIX .. "/arm-none-eabi-objcopy"

-- Check if a file exists --
function FileExists(strFileName)
	local fileHandle, strError = io.open(strFileName, "r")
	if fileHandle ~= nil then
		io.close(fileHandle)
		return true
	else
		if string.match(strError, "No such file or directory") then
			return false
		else
			return true
		end
	end
end

if (FileExists(GCC_ARM_CC_PATH) == false) then
	error("Compiler does not exist, did you install the toolchain? Please run bin/update_sdk.sh.")
end

table.insert(premake.option.list["platform"].allowed, { "arm" })

premake.platforms.arm = {
	cfgsuffix = "arm",
	iscrosscompiler = true
}

table.insert(premake.fields.platforms.allowed, "arm")


if(_OPTIONS.platform == 'arm') then
	premake.gcc.cc = GCC_ARM_CC_PATH
	premake.gcc.cxx = GCC_ARM_CPP_PATH
	premake.gcc.ar = GCC_ARM_AR_PATH
end
