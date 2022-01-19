
project('cumbia-tango-cached-dbconf', 'cpp', version : '1.0.0',
    default_options : ['c_std=c17', 'cpp_std=c++17'])

project_description = 'Use Tango attribute configuration cached on a Redis database before accessing the Tango Database. Client library. Needs
ca-tango-cache-db-mgr service and redis'

cu_version = meson.project_version() # set in project() below
ver_arr = cu_version.split('.')

cu_major_version = ver_arr[0]
cu_minor_version = ver_arr[1]
cu_micro_version = ver_arr[2]

conf = configuration_data()
# Surround the version in quotes to make it a C string
conf.set_quoted('VERSION', cu_version)
configure_file(output : 'config.h',
               configuration : conf)

# enable pinfo cuprintf
cpp_arguments = [ '-DCUMBIA_DEBUG_OUTPUT=1', '-DCUTANGODBCACHED_DBCONF_VERSION="' + cu_version + '"' ]

openssldep = dependency('openssl')
curldep = dependency('libcurl')
# for activities / threads
cumbiadep = dependency('cumbia', version: '>=1.4.0')
cumbiatangodep = dependency('cumbia-tango', version: '>=1.4.0')
redisdep = dependency('redis++')

nlohmann_json_dep = dependency('nlohmann_json', required : false)
if not nlohmann_json_dep.found()
    message('nlohmann_json installation not found: trying subproject method')
	message('refer to https://gitlab.com/PWMA/canone3/-/blob/master/nchan-setup/ubuntu-install.md')
	# nlohmann_json dependency is added as subdir 'nlohmann-json-include'
	nlohmann_json_proj = subproject('nlohmann-json-include')
	nlohmann_json_dep = nlohmann_json_proj.get_variable('nlohmann_json_dep')
endif

# tango library
tangodep = dependency('tango')

includedirs = include_directories('src')

deps = [ dependency('threads'),
    openssldep,
    tangodep,
	nlohmann_json_dep,
	curldep,
	cumbiadep,
        cumbiatangodep,
	redisdep ]

headers = [
        'src/config.h',
        'src/cutacached-dbconf-executor.h',
                'src/cutadbcjsoniz.h',
                'src/cutadbcache-curl.h',
                'src/cutadbcacheu.h',
                'src/cutdbredis-service.h',
                'src/cutcachedconffactories.h'
]

sources = [
        'src/cutacached-dbconf-executor.cpp',
                'src/cutadbcjsoniz.cpp',
                'src/cutadbcache-curl.cpp',
                'src/cutadbcacheu.cpp',
                'src/cutdbredis-service.cpp',
                'src/cutcachedconffactories.cpp'
		]

cutgcached_dbconflib = shared_library('cumbia-tango-cached-dbconf',
    sources, dependencies : deps, cpp_args : cpp_arguments,
	include_directories : includedirs,
	install : true)

inst_h = ['src/cutacached-dbconf-executor.h',
    'src/cutdbredis-service.h',
    'src/cutcachedconffactories.h' ]

install_headers(inst_h, subdir : 'cumbia-tango-cached-dbconf')

### ====================================== pkg config   ============================
pkgconfig = find_program('pkg-config', required: false)
if not pkgconfig.found()
  error('MESON_SKIP_TEST: pkg-config not found')
endif

pkgg = import('pkgconfig')

h = ['cumbia-tango-cached-dbconf']  # subdirectories of ${prefix}/${includedir} to add to header path
pkgg.generate(
    libraries : cutgcached_dbconflib,
    subdirs : h,
    version : meson.project_version(),
    name : 'libcumbia-tango-cached-dbconf',
    filebase : 'cumbia-tango-cached-dbconf',
    install_dir : 'lib/pkgconfig',
    description : project_description )


### ====================================== documentation ===========================
doxygen = find_program('doxygen', required : false)
if not doxygen.found()
  error('MESON_SKIP_TEST doxygen not found.')
endif
  doxydata = configuration_data()
  doxydata.set('VERSION', meson.project_version())


if find_program('dot', required : false).found()
  # In the real world this would set the variable
  # to YES. However we set it to NO so that the
  # list of generated files is always the same
  # so tests always pass.
  doxydata.set('HAVE_DOT', 'YES')
else
  doxydata.set('HAVE_DOT', 'NO')
endif

message('\n\033[1;32m*\n*\033[0m enable subdir(\'doc\') in meson.build to generate documentation\n\033[1;32m*\033[0m\n')
# subdir('doc')



