#!/usr/bin/env texlua
-----------------------------------------------------------------------
--         FILE:  xindex.lua
--  DESCRIPTION:  create an index
-- REQUIREMENTS:  
--       AUTHOR:  Herbert Voß
--      LICENSE:  LPPL 1.3
--
-- $Id: xindex.lua 21 2022-01-27 17:45:10Z hvoss $
-----------------------------------------------------------------------

        xindex = xindex or { }
 local version = 0.36
xindex.version = version
--xindex.self = "xindex"

--[[doc--

xindex(1)

This file is provided under the terms of the LPPL v1.3 or
later as printed in full text in the manual (xindex.pdf).

\url{https://ctan.org/license/lppl1.3}.

Report bugs to

    \url{https://gitlab.com/hvoss49/xindex/issues}.

--doc]]--

kpse.set_program_name("luatex")

local f = kpse.find_file("lualibs.lua")
--print ("filename "..f)

require("lualibs")  -- all part of LuaTeX
require('unicode')
require('string')
require("lpeg")

local args = require ('xindex-lapp') [[
  parameter handling
    -q,--quiet
    -h,--help
    -v...          Verbosity level; can be -v, -vv, -vvv
    -c,--config (default cfg)
    -e,--escapechar (default ")
    -n,--noheadings 
    -a,--no_casesensitive
    -b,--no_labels
    -o,--output (default "")
    -l,--language (default en)
    -p,--prefix (default L)
    -u,--use_UCA
    -s,--use_stdin
    <files...> (default stdin) .idx file(s)
]]

--[[
    No -v flag, v is just { false }. not args.v[1] is true, so vlevel becomes 0.
    One -v flags, v is { true }
    Two -v flags, v is { true, true }
    Three -v flags, v is { true, true, true } 
]]

vlevel = not args.v[1] and 0 or #args.v
not_quiet = not args["quiet"]

local luaVersion = _VERSION
if (luaVersion < "Lua 5.3") then
  print("=========================================")
  print("Sorry. but we need at least LuaTeX 1.09")
  print("Leaving program xindex")
  print("=========================================")
  os.exit()
end

useStdInput = args["use_stdin"]

--local inspect = require 'inspect' 
--print(inspect(args))

--[[
if args.h then
print(
Syntax: xinput [options] <file>
By default the Lua program "xindex" creates a so-called
.ind file, which has the same main filename as the input file
unless you are using the option "-o <output file>"  There will 
be no log file created. 
)
end
]]

require('xindex-baselib')

local nInFiles = #args.files
if not useStdInput then
  --print(tostring(nInFiles).." input files are given!")
  inFiles = {}    --args.files as strings
  for i = 1,nInFiles do
    local file = args.files_name[i]
    if not file_exists(file) then
      if file_exists(file..".idx") then
        inFiles[#inFiles+1] = file..".idx"
      end
    else
      inFiles[#inFiles+1] = file
    end
  end  
end

-- print ("Check Logfile:")

outfilename = ""
logfilename = ""

if args["output"] == '""' then
  if not useStdInput then
    if inFiles[1]:sub(inFiles[1]:len()-3,inFiles[1]:len()) == ".idx" then 
      outfilename = inFiles[1]:sub(1,inFiles[1]:len()-3).."ind"
      if nInFiles > 1 then
        logfilename = "xindex.ilg"
      else 
        logfilename = inFiles[1]:sub(1,inFiles[1]:len()-3).."ilg"
      end
    else
      outfilename = inFiles[1]..".ind"
      if nInFiles > 1 then
        logfilename = "xindex.ilg"
      else 
        logfilename = inFiles[1]..".ilg"
      end
    end
  else
    outfilename = "xindex.ind"
    logfilename = "xindex.ilg"
  end
else
  outfilename = args.output
  if nInFiles > 1 or useStdInput then
    logfilename = "xindex.ilg"
  else 
    logfilename = outfilename:gsub('%p...','')..".ilg"
  end
end



logFile = io.open(logfilename,"w+")
require('xindex-lib')

writeLog(2,"xindex v."..version.." (c) Herbert Voß\n",-1)
writeLog(1,"Verbose level = "..vlevel.."\n",1)
writeLog(2,"Logfile:"..logfilename,1)

writeLog(2,"Open outputfile "..outfilename,0)
outFile = io.open(outfilename,"w+")
writeLog(2," ... done\n",0)

if vlevel > 0 then
  writeLog(1,"---------- parameter ----------\n",1)
  for k,v in pairs(args) do
    writeLog(1,tostring(k)..", "..tostring(v).."\n",1)
  end
  for k=1,#args.v do 
    writeLog(1,"v["..k.."]= "..tostring(args.v[k]).."\n",1) 
  end
  writeLog(1,"---------- parameter ----------\n",1)
end

-- writeLog(2,"Using input file: "..inFile.."\n",0)

labelPrefix = args.prefix
writeLog(2,"Label prefix: "..labelPrefix.."\n",-1)

writeLog(2,"Loading common config file ".."xindex-cfg-common\n",1)
Config_File_Common = kpse.find_file("xindex-cfg-common.lua") 
cfg_common = require(Config_File_Common)

local user_config_file = "xindex-"..args["config"]..".lua"
print("Local config file is: "..user_config_file)
writeLog(2,"Loading local config file "..user_config_file,0)
if kpse.find_file(user_config_file) 
  then Config_File = kpse.find_file(user_config_file) 
  else print("Cannot find config file with kpse.find_file!!")
end
print("\nLocal KPSE config file is: "..Config_File.."\n")
cfg = require(Config_File)
writeLog(2," ... done\n",0)

-- Create the character list maps for faster sorting

alphabet_lower_map = CreateCharListMap(alphabet_lower)
alphabet_upper_map = CreateCharListMap(alphabet_upper)

local esc_char = args.escapechar
writeLog(2,"Escapechar = "..esc_char.."\n",1)
escape_chars = { -- by default " is the escape char
  {esc_char..'"', '//escapedquote//',     '"'    },
  {esc_char..'@', '//escapedat//',        '@'    },
  {esc_char..'|', '//escapedvert//',      '|'    },
  {esc_char..'!', '//escapedexcl//',       '!'    }
--  {esc_char..'%(', '//escapedparenleft//', '('    },  -- ( must beescaped
--  {esc_char..'%)', '//escapedparenright//',')'    }   -- )  "      "
}

-- esc_char..'%( is not needed because it can only appear after |

language = "en" -- default language

language = string.lower(args["language"]):sub(1, 2)
writeLog(2,"Language = "..language.."\n",1) 
if (indexheader[language] == nil) then
  writeLog(2,'Corrected the unknown language "'..language..'" to "en"'.."\n",0) 
  language = "en"
end  
index_header = indexheader[language]
if vlevel > 0 then for i=1,#index_header do writeLog(2,index_header[i].."\n",1) end end
if (folium[language] == nil) then
  writeLog(2,'Corrected the unknown language "'..language..'" for page folium to "en"'.."\n",0) 
  page_folium = folium["en"]
else
  page_folium = folium[language]
end  

use_UCA = args["use_UCA"]
if use_UCA then
  writeLog(1,"Will use LUA-UCA\n",1)
  ducet = require "lua-uca.lua-uca-ducet"
  collator = require "lua-uca.lua-uca-collator"
  languages = require "lua-uca.lua-uca-languages"
  collator_obj = collator.new(ducet)
  
  local uca_config_file = "xindex-cfg-uca.lua"
  writeLog(2,"Loading local UCA config file "..uca_config_file,0)
  UCA_Config_File = kpse.find_file(uca_config_file) 
  uca_cfg = require(UCA_Config_File)
  writeLog(2," ... done\n",0)
  
-- language name specified on the command line doesn't seem to be available
-- in the config file, so we just try to find it ourselves
  for i, a in ipairs(arg) do
    if a == "-l" or a=="--language" then
      language = arg[i+1]
      break
    end
  end

  if languages[language] then
    print("[Lua-UCA] Loading language: " .. language)
    collator_obj = languages[language](collator_obj)
  end
else
  writeLog(1,"Will _not_ use LUA-UCA\n",1)
end

upper = unicode.utf8.upper

no_caseSensitive = args["no_casesensitive"]
if no_caseSensitive then
  writeLog(1,"Sorting will be no case sensitive\n",1)
else
  writeLog(1,"Sorting will be case sensitive\n",1)
end

no_headings = args["noheadings"]
if no_headings then
  writeLog(1,"Output with NO headings between different first letter\n",1)
else
  writeLog(1,"Output with headings between different first letter\n",1)
end

no_labels = args["no_labels"]
if no_headings then
  writeLog(1,"Index without labels\n",1)
else
  writeLog(1,"Index with labels\n",1)
end

writeLog(2,"Open outputfile "..outfilename,0)
outFile = io.open(outfilename,"w+")
writeLog(2,"... done\n",0)

writeLog(1,"Starting base file ... \n",2)

BaseRunFile = kpse.find_file("xindex-base.lua") 
dofile(BaseRunFile)

logFile:close()

