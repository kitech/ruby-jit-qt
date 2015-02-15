#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

### write your test code here
# test delete of Qt object
# 对象的回收与动态析构,主要是C++类的delete操作。

def make_garbage(n)
    n.times {
        s = Qt5::QString.new
    }
end

def make_garbage2(n)
    n.times {
        t = Qt5::QThread.new
    }
end

def make_garbage3(n)
    n.times {
        t = Qt5::QApplication.new(ARGV.count, ARGV)
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
    n = 1
    make_garbage(n)

    dump_gc_stat
    # make_newobj
    sleep(0.1)
    GC.start(full_mark: true, immediate_sweep: true)
    p "Should see sth. like this(#{n} lines):"
    p '    RubyInit::Qt_class_dtor - QString flase'
    sleep(1)

    #######
    make_garbage2(n)
    sleep(0.1)
    GC.start(full_mark: true, immediate_sweep: true)
    p "Should see sth. like this(#{n} lines):"
    p '    RubyInit::Qt_class_dtor - QThread flase'

    #######
    make_garbage3(n)
    sleep(0.1)
    GC.start(full_mark: true, immediate_sweep: true)
    p "Should see sth. like this(#{n} lines):"
    p '    RubyInit::Qt_class_dtor - QApplication flase'

end

main()

