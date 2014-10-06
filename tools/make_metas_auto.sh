#!/bin/sh

metalines=$(grep -E "const QMetaObject y" moc_*.cpp | awk '{print $3}')
auto_file=metalize/metas_auto.cpp

echo "// only #include by metas.cpp's init_class_metas function" > $auto_file
echo "// auto generated file, on `date`" >> $auto_file

echo "// void init_class_metas() { " >> $auto_file

for metaline in $metalines ; do
    echo $metaline;
    klass_name=$(echo $metaline | awk -F: '{print $1}')
    echo $klass_name;
    code_line="    __rq_metas[\"$klass_name\"] = &$metaline;";
    echo $code_line;
    echo $code_line >> $auto_file;
done

echo "// }" >> $auto_file
