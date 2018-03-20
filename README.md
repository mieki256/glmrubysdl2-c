glmrubysdl2-c
=============

C + SDL2 + mruby sample.

Description
-----------------

fork [SDL_imageとmrubyを使ってみた - mirichiの日記](http://d.hatena.ne.jp/mirichi/20120816/p1)


Requirement
-----------------

* Windows10 x64
* [MSYS2](https://www.msys2.org/)
* [SDL2](https://www.libsdl.org/)
* [mruby/mruby](https://github.com/mruby/mruby)
* [iij/mruby-require](https://github.com/iij/mruby-require)


Build
-----------------

    ~/prg
    |
    +---- mruby/
          |
          +---- mruby/
          |
          +---- glmrubysdl2-c/

### Instal gcc, SDL2, and Ruby

Start "MSYS2 MinGW 32-bit (32bit)"

#### Install gcc

    $ pacman -S base-devel
    $ pacman -S pacman -S mingw-w64-i686-toolchain

#### Install SDL2

    $ pacman -Ss SDL2 i686
    $ pacman -S mingw-w64-i686-SDL2
    $ pacman -S mingw-w64-i686-SDL2_image
    $ pacman -S mingw-w64-i686-smpeg2
    $ pacman -S mingw-w64-i686-SDL2_mixer
    $ pacman -S mingw-w64-i686-SDL2_net
    $ pacman -S mingw-w64-i686-SDL2_ttf
    $ pacman -S mingw-w64-i686-SDL2_gfx

#### Install Ruby

    pacman -S ruby

#### Check.

    $ gcc --version
    gcc.exe (Rev1, Built by MSYS2 project) 7.3.0
    Copyright (C) 2017 Free Software Foundation, Inc.

    $ bison --version
    bison (GNU Bison) 3.0.4

    $ sdl2-config --version
    2.0.8

    $ sdl2-config --cflags
    -I/mingw32/include/SDL2 -Dmain=SDL_main
    
    $ sdl2-config --libs
    -L/mingw32/lib -lmingw32 -lSDL2main -lSDL2 -mwindows
    
    $ ruby --version
    ruby 2.4.0p0 (2016-12-24 revision 57164) [i386-msys]


### Build mruby + mruby-require

#### Doenload mruby

    $ cd ~
    $ mkdir prg
    $ cd prg
    $ mkdir mruby
    $ cd mruby
    $ git clone https://github.com/mruby/mruby.git
    $ cd mruby

#### Edit build_config.rb

    MRuby::Build.new do |conf|
    ...
      conf.cc.command = 'i686-w64-mingw32-gcc'
      conf.linker.command = 'i686-w64-mingw32-gcc'
      conf.gem :github => 'iij/mruby-require'
    end
    
    MRuby::Build.new('host-debug') do |conf|
    ...
      conf.cc.command = 'i686-w64-mingw32-gcc'
      conf.linker.command = 'i686-w64-mingw32-gcc'
      conf.gem :github => 'iij/mruby-require'
    end
    
    MRuby::Build.new('test') do |conf|
    ...
      conf.cc.command = 'i686-w64-mingw32-gcc'
      conf.linker.command = 'i686-w64-mingw32-gcc'
      conf.gem :github => 'iij/mruby-require'
    end

#### Edit mruby/mrbgems/mruby-io/mrbgem.rake

    when /mingw|mswin/

->

    when /mingw|mswin|msys/

#### Build mruby

    $ make

Check.

    $ ./bin/mruby --version
    mruby 1.4.0 (2018-1-16)


### Build glmrubysdl2-c

#### Download

    $ cd ~/prg/mruby/
    $ git clone https://github.com/mieki256/glmrubysdl2-c.git
    $ cd glmrubysdl2-c

#### Build

    $ make


Run
-----------------

    $ ./glmrubysdl2.exe

