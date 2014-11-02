#!/bin/sh

MODULES="QtCore QtGui QtWidgets QtNetwork"
QTINC_BASE=/usr/include/qt
CXXFLAGS="-fPIC -I${QTINC_BASE}"


for module in $MODULES ; do
    echo $module;
    CXXFLAGS="${CXXFLAGS} -I${QTINC_BASE}/${module}"
done

echo $CXXFLAGS

QSslConfiguration


Holds the configuration and state of an SSL connection

QSslError


SSL error

QSslKey

care_classes="QAccessible QAction QActionGroup QObject QString QBitArray
QBitmap QBuffer QBoxLayout
QByteArray QThread QCoreApplication
QChar QStringList
QTimer QDateTime QUrl QFile QRegExp
QHostAddress QAbstractSocket QTcpSocket QTcpServer
QSslSocket QSslCipher QSslConfiguration QSslError QSslKey
QNetworkCookie QNetworkCookieJar
QNetworkAccessManager QNetworkRequest QNetworkReply
QGuiApplication  QIcon
QWidget QMainWindow
QPushButton 
QApplication
";

# 32 位系统有问题
broken_classes="QNetworkReply"


##### gen jit types body
jit_types_body_file="metalize/jit_types_body.cpp"
qtruby_register_file="metalize/qtruby_auto_body.cpp"
echo "// auto generated" > $jit_types_body_file
echo "// auto generated" > $qtruby_register_file
for klass in $care_classes ; do
    echo "aaa $klass bbb"
    echo "(void)(Ya${klass}*)v0;" >> $jit_types_body_file
    echo "(void)(${klass}*)v0;" >> $jit_types_body_file
    echo "RQCLASS_REGISTER(${klass});" >> $qtruby_register_file
done


#### exit

meta_objects_file=metalize/metar_objects.cpp
echo "" > $meta_objects_file;
metar_proto_file=metalize/metar_protos.cpp
echo "" > $metar_proto_file

for module in $MODULES ; do
    lower_module=${module,,}  # lower it
    lower_module=${lower_module:2}    #substr it
    packed_header_file=$QTINC_BASE/$module/$module
    echo $packed_header_file
    header_files=$(grep "#include " $packed_header_file | awk -F\" '{print $2}')
    class_header_files=$(ls $QTINC_BASE/$module/ | grep -v "\.h")

    ### 输出到metar_classes
    metar_classes_header="./metalize/metar_classes_qt${lower_module}.h"
    metar_classes_cpp="./metalize/metar_classes_qt${lower_module}.cpp"
    echo -e "#include \"metar_classes_qt${lower_module}.h\"\n" > $metar_classes_cpp
    echo -e "#ifndef _METAR_CLASSES_${lower_module^^}_H_" > $metar_classes_header
    echo -e "#define _METAR_CLASSES_${lower_module^^}_H_\n" >> $metar_classes_header

    for header_file in $header_files ; do
        # echo $header_file;

        header_path=$QTINC_BASE/$module/$header_file
        yheader_file="y${header_file}"
        yheader_path="./metalize/${lower_module}/${yheader_file}"
        ycpp_path="./metalize/${lower_module}/${yheader_file}.cpp"
        yproto_path="./metalize/${lower_module}/${yheader_file}.proto.cpp"
        # echo $yheader_path
        yheader_stderr_file="${yheader_path}.err";
        # yheader_moc_file="./moc/${lower_module}/moc_${yheader_file}.cpp"

        ### resolve class name
        klass_name=
        for class_header_file in $class_header_files ; do
            if [ "${class_header_file,,}.h" == "$header_file" ] ; then
                klass_name=$class_header_file
                break;
            fi
        done
        if [ x"$klass_name" == x"" ] ; then
            echo "can not find real class name for header: ${header_file}";
            continue;
        fi

        if [ x"${header_file:0:9}" == x"qabstract" ] ; then
            if [ x"${klass_name}" == x"QAbstractSocket" ] ; then
                true
            else
                echo "omit qabstractxxx class.";
                continue;
            fi
        fi

        if [ x"$klass_name" == x"QGlobalStatic" ] \
                || [ x"$klass_name" == x"QProcess" ] \
                || [ x"$klass_name" == x"QException" ] \
                || [ x"$klass_name" == x"QFutureInterface" ] \
                ; then
            continue;
        fi

        # fix QFlag not match qflag.h
        if [ x"$klass_name" == x"QFlags" ] ; then
            klass_name="QFlag"
        fi

        in_care=0
        for care_class in $care_classes ; do
            if [ x"$klass_name" == x"$care_class" ] ; then
                in_care=1
                break
            fi
        done

        if [ x"$in_care" == x"0" ] ; then
            continue;
        fi
        
        # klass_name="${header_file:1}"
        # klass_name="Q${klass_name^}"
        # klass_name=$(echo $klass_name | awk -F. '{print $1}')
        # # echo $klass_name
        # if [ ! -e "$QTINC_BASE/$module/$klass_name" ] ; then
        #     # echo "class name error.${klass_name}";
        #     new_klass_name=$(ls -lh $QTINC_BASE/$module|grep -i $klass_name|grep -v ".h"|awk '{print $9}')
        #     if [ x"$new_klass_name" = x"" ] ; then
        #         echo "can not find class name: ${klass_name}";
        #     else
        #         echo "correct class name: ${klass_name} ==> ${new_klass_name}"
        #         klass_name=$new_klass_name;
        #     fi
        # fi

        # continue;

        
        echo "./demos/qtparser $module  $klass_name $header_path"
        ./demos/qtparser "$module"  "$klass_name" "$header_path" 2>$yheader_stderr_file 1>/dev/null
        mv -v  ./ya${klass_name,,}.h $yheader_path
        mv -v  ./ya${klass_name,,}.cpp $ycpp_path
        mv -v ./ya${klass_name,,}.proto.cpp $yproto_path
        echo $yheader_path
        echo $yheader_stderr_file

        yheader_content=$(cat $yheader_path);
        yheader_content="#ifndef _YA${klass_name^^}_H_\n#define _YA${klass_name^^}_H_\n"
        echo -e $yheader_content > "${yheader_path}.tmp"
        cat $yheader_path >> "${yheader_path}.tmp"
        echo -e "\n\n#endif\n\n" >> "${yheader_path}.tmp"
        mv -v "${yheader_path}.tmp" $yheader_path

        # omit manual moc generator, cmake can do it automatically
        # moc $yheader_path > $yheader_moc_file
        # echo $yheader_moc_file

        # out to there
        echo -e "#include \"$yheader_path\"\n" >> $metar_classes_header
        echo -e "#include \"$ycpp_path\"\n" >> $metar_classes_cpp
        echo -e "#include \"$yproto_path\"\n" >> $metar_proto_file

        # echo -e "#include \"${lower_module}/moc_y${klass_name,,}.cpp\"\n" >> $metar_classes_cpp

        # out to meta_objects.cpp
        # echo -e "__rq_metas[\"y${klass_name}\"] = &y${klass_name}::staticMetaObject;" >> $meta_objects_file

        # exit;
    done  # end for header_file
    
    echo -e "#endif\n\n" >> $metar_classes_header
    
done  # end for module

### generator ast file
ast_file="./data/qthdrsrc.ast"
# -DQ_NO_TYPESAFE_FLAGS
clang++ -x c++ -S -emit-ast "./qthdrsrc.h" -fPIC -I. -I/usr/include/qt -I/usr/include/qt/QtCore \
    -I/usr/include/qt/QtGui -I/usr/include/qt/QtWidgets -I/usr/include/qt/QtNetwork
mv -v "qthdrsrc.ast" "${ast_file}"

### generator jit_types.ll
clang++ -S -emit-llvm metalize/jit_types.cpp -I. -I/usr/include/qt/QtCore/ -I/usr/include/qt \
    -fPIC -std=c++11 -I/usr/include/qt/QtGui -I/usr/include/qt/QtWidgets -I/usr/include/qt/QtNetwork
mv -v jit_types.ll metalize/jit_types.ll

