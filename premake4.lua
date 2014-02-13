--[[
	Premake script to create the makefiles for Erolf
--]]

dofile "premake4_arm.lua"


-- Erolf solution config
solution "Erolf"
	configurations { "Debug", "Release", "Processor1", "Processor2" }
	platforms { "native", "arm" }

	project "erolf"
		kind "ConsoleApp"
		language "c"

		includedirs {"src/", "src/framework/include/", "src/target/include"}

		-- global compiler options
		flags {
			
		}
		
		-- extra options
		newoption {
		   trigger     = "component",
		   value       = "nr",
		   description = "Choose target component",
		   allowed = {
		      { "p1",  "Processor 1" },
		      { "p2",  "Processor 2" },
		      { "vector",  "VectorTableLib" }
		   }
		}

		buildoptions { "-Wall", "-Wextra", "-Werror", "-std=c99", "-O2"}
		linkoptions {
			"-L."
		}

		local targetDir        = "build/".._OPTIONS.platform.."/"
		local targetDirRelease = targetDir.."release/".._OPTIONS["component"].."/"
		local targetDirDebug   = targetDir.."debug/".._OPTIONS["component"].."/"
		local targetName       = "erolf"
		local targetSuffix     = ".elf"

	
		targetname(targetName..targetSuffix)
		
			
		--[[
			global config
		--]]

		prebuildcommands { '@echo "\\n\\n--- Starting to build: `date` ---\\n\\n"' }
		postbuildcommands { '@echo "\\n\\n--- Finished build ---\\n\\n"' }

		configuration "DEBUG"
			--[[
			--  valid DEBUG SYMBOLS:
			--    DEBUG
			--]]
			defines { "DEBUG", "STM32F4" }
			flags { "Symbols"}
			objdir(targetDirDebug.."/obj/")
			buildoptions {"-g3", "-O0"}
			
			
		configuration "RELEASE"
			flags { "Symbols" }
			defines { "RELEASE", "STM32F4" }
			objdir(targetDirRelease.."/obj/")
			buildoptions {"-g3", "-O3"}
			
		--[[
			processor location 
		--]]
		
		configuration "vector"
			kind "StaticLib"
			files {
				"src/target/stm32f4/vectorISR.c"
			}
			linkoptions {
			}
			targetname("vector")
			
		configuration "p1"
			files {
				"src/**.c"
			}
			excludes {
				"src/target/stm32f4/p2/**.c",
				"src/application/p2/**.c",
				"src/target/stm32f4/vectorISR.c"
			}
			linkoptions {
				"-Tsrc/target/stm32f4/stm32.ld",
				"-lvector",
				"-lm",
			--	"-lc",
				"-Wl,-Map=erolfP1.map,--cref"
			}
			targetname("erolfP1"..targetSuffix)
			
		configuration "p2"			
			files {
				"src/**.c"
			}
			excludes {
				"src/target/stm32f4/p1/**.c",
				"src/application/p1/**.c",
				"src/target/stm32f4/vectorISR.c"
			}
			
			linkoptions {
				"-Tsrc/target/stm32f4/stm32.ld",
				"-lvector",
				"-lm",
			--	"-lc",
				"-Wl,-Map=erolfP2.map,--cref"
			}
			
			targetname("erolfP2"..targetSuffix)
			

		--[[
			ARM specific config	
		--]]
		configuration "arm"
			defines { "STM32F4"
				--   ,"NO_LOG_OUTPUT" 
					}
			
			buildoptions { "-mthumb", "-mcpu=cortex-m4", "-msoft-float" }
			linkoptions { "-mthumb -mcpu=cortex-m4 -msoft-float" }
			linkoptions {
				"-mfix-cortex-m3-ldrd",
				"-nostartfiles",
			}

