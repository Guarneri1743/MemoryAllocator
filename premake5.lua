local solution_dir = "solution/" .. _ACTION

function setupIncludeDirs()
   includedirs {
      "src"
   }
end

function setupSlotion()
   location(solution_dir)
   solution "MemoryAllocator"
      configurations {
         "Debug", 
         "Release"
      }

      platforms { "Win64" }
      warnings "Extra"
      floatingpoint "Fast"
      symbols "On"
      cppdialect "C++11"
      rtti "On"
      characterset ("MBCS")

      configuration "Debug*"
         defines { "DEBUG", "_DEBUG" }
         targetdir ( solution_dir .. "lib/Debug" )

      configuration "Release*"
         defines { "NDEBUG" }
         optimize "On"
         targetdir ( solution_dir .. "lib/Release"  )

end

function setupTestProject()
   project "MemoryAllocatorTest"
   kind "ConsoleApp"
   language "C++"

    files { 
      "src/detail/*.*", 
      "src/*.*",
      "test/*.*"
   }


   filter { "configurations:Debug*" }
      targetdir (solution_dir .. "/bin/Debug")

   filter { "configurations:Release*" }
      targetdir (solution_dir .. "/bin/release")
end

setupIncludeDirs()
setupSlotion()
setupTestProject()