#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

### write your test code here
# test gc of Qt object
# 对象的回收与动态析构。

def make_garbage(n)
    n.times {
        s = Qt5::QString.new
    }
end

def dump_gc_stat()
    #p GC.count
    #p GC.malloc_allocated_size
    #p GC.malloc_allocations
    #p GC.stat
    #p GC.latest_gc_info
end

$garr = []
$gidx = 0
def make_newobj()
    10990.times {
        $garr[$gidx] = rand()
        $gidx += 1
        # s = NeedGc.new

        #$garr[$gidx] = s
        #$gidx += 1

        #s2 = NeedGc.new
        s3 = Qt5::QString.new
    }
    p 'garr size:' + $garr.count.to_s
end

def main()
    GC.start(full_mark: true, immediate_sweep: true)
    
    dump_gc_stat
    n = (rand()*1000 % 10).to_i + 1
    make_garbage(n)

    dump_gc_stat
    # make_newobj
    sleep(0.1)
    GC.start(full_mark: true, immediate_sweep: true)
    p "Should see sth. like this(#{n} lines):"
    p '    RubyInit::Qt_class_dtor - 0x222eff0 Qt5::QString'
    sleep(1)
end

main()

