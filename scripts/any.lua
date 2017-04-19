return function(link_type, defs)
	defs = defs or {}
	local name = "any-vm"
	if link_type == "SharedLib" then 
		name = name .. "_shared" 
	end
	project(name)
		language("C++")
		uuid(os.uuid(name))
		kind(link_type)
		if link_type == SharedLib then
			defines { "ANY_EXPORT", "ANY_SHARED" }
			configuration { "mingw*" }
				linkoptions { "-shared" }
			configuration { "linux-*" }
				buildoptions { "-fPIC" }
			configuration {}
		end
		defines { defs }
		includedirs {
			DIR.INCLUDE,
			path.join(DIR.PRIVATE, "any")
		}
		files {
			path.join(DIR.INCLUDE, "any/**.h"),
			path.join(DIR.PRIVATE, "any/**.h"),
			path.join(DIR.PRIVATE, "any/**.c")
		}
end