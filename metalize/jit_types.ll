; ModuleID = 'metalize/jit_types.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.QArrayData = type { %"class.QtPrivate::RefCount", i32, i32, i64 }
%"class.QtPrivate::RefCount" = type { %class.QBasicAtomicInteger }
%class.QBasicAtomicInteger = type { i32 }
%class.YaQAccessible = type { i8 }
%class.QAccessible = type { i8 }
%class.YaQAction = type { %class.QAction }
%class.QAction = type { %class.QObject }
%class.QObject = type { i32 (...)**, %class.QScopedPointer }
%class.QScopedPointer = type { %class.QObjectData* }
%class.QObjectData = type { i32 (...)**, %class.QObject*, %class.QObject*, %class.QList, i32, i32, %struct.QDynamicMetaObjectData* }
%class.QList = type { %union.anon }
%union.anon = type { %struct.QListData }
%struct.QListData = type { %"struct.QListData::Data"* }
%"struct.QListData::Data" = type { %"class.QtPrivate::RefCount", i32, i32, i32, [1 x i8*] }
%struct.QDynamicMetaObjectData = type opaque
%class.YaQActionGroup = type { %class.QActionGroup }
%class.QActionGroup = type { %class.QObject }
%class.YaQObject = type { %class.QObject }
%class.YaQString = type { %class.QString }
%class.QString = type { %struct.QTypedArrayData* }
%struct.QTypedArrayData = type { %struct.QArrayData }
%class.YaQBitArray = type { %class.QBitArray }
%class.QBitArray = type { %class.QByteArray }
%class.QByteArray = type { %struct.QTypedArrayData.0* }
%struct.QTypedArrayData.0 = type { %struct.QArrayData }
%class.YaQBitmap = type { %class.QBitmap }
%class.QBitmap = type { %class.QPixmap }
%class.QPixmap = type { %class.QPaintDevice, %class.QExplicitlySharedDataPointer }
%class.QPaintDevice = type { i32 (...)**, i16, %class.QPaintDevicePrivate* }
%class.QPaintDevicePrivate = type opaque
%class.QExplicitlySharedDataPointer = type { %class.QPlatformPixmap* }
%class.QPlatformPixmap = type opaque
%class.YaQBuffer = type { %class.QBuffer }
%class.QBuffer = type { %class.QIODevice }
%class.QIODevice = type { %class.QObject }
%class.YaQBoxLayout = type { %class.QBoxLayout.base, [4 x i8] }
%class.QBoxLayout.base = type { %class.QLayout.base }
%class.QLayout.base = type <{ %class.QObject, %class.QLayoutItem.base }>
%class.QLayoutItem.base = type <{ i32 (...)**, %class.QFlags }>
%class.QFlags = type { i32 }
%class.QBoxLayout = type { %class.QLayout.base, [4 x i8] }
%class.YaQByteArray = type { %class.QByteArray }
%class.YaQThread = type { %class.QThread }
%class.QThread = type { %class.QObject }
%class.YaQCoreApplication = type { %class.QCoreApplication }
%class.QCoreApplication = type { %class.QObject }
%class.YaQChar = type { %class.QChar }
%class.QChar = type { i16 }
%class.YaQStringList = type { %class.QStringList }
%class.QStringList = type { %class.QList.1 }
%class.QList.1 = type { %union.anon.2 }
%union.anon.2 = type { %struct.QListData }
%class.YaQTimer = type { %class.QTimer.base, [3 x i8] }
%class.QTimer.base = type <{ %class.QObject, i32, i32, i32, i8 }>
%class.QTimer = type { %class.QObject, i32, i32, i32, i8 }
%class.YaQDateTime = type { %class.QDateTime }
%class.QDateTime = type { %class.QExplicitlySharedDataPointer.3 }
%class.QExplicitlySharedDataPointer.3 = type { %class.QDateTimePrivate* }
%class.QDateTimePrivate = type opaque
%class.YaQUrl = type { %class.QUrl }
%class.QUrl = type { %class.QUrlPrivate* }
%class.QUrlPrivate = type opaque
%class.YaQFile = type { %class.QFile }
%class.QFile = type { %class.QFileDevice }
%class.QFileDevice = type { %class.QIODevice }
%class.YaQRegExp = type { %class.QRegExp }
%class.QRegExp = type { %struct.QRegExpPrivate* }
%struct.QRegExpPrivate = type opaque
%class.YaQHostAddress = type { %class.QHostAddress }
%class.QHostAddress = type { %class.QScopedPointer.4 }
%class.QScopedPointer.4 = type { %class.QHostAddressPrivate* }
%class.QHostAddressPrivate = type opaque
%class.YaQAbstractSocket = type { %class.QAbstractSocket }
%class.QAbstractSocket = type { %class.QIODevice }
%class.YaQTcpSocket = type { %class.QTcpSocket }
%class.QTcpSocket = type { %class.QAbstractSocket }
%class.YaQTcpServer = type { %class.QTcpServer }
%class.QTcpServer = type { %class.QObject }
%class.YaQSslSocket = type { %class.QSslSocket }
%class.QSslSocket = type { %class.QTcpSocket }
%class.YaQSslCipher = type { %class.QSslCipher }
%class.QSslCipher = type { %class.QScopedPointer.5 }
%class.QScopedPointer.5 = type { %class.QSslCipherPrivate* }
%class.QSslCipherPrivate = type opaque
%class.YaQSslConfiguration = type { %class.QSslConfiguration }
%class.QSslConfiguration = type { %class.QSharedDataPointer }
%class.QSharedDataPointer = type { %class.QSslConfigurationPrivate* }
%class.QSslConfigurationPrivate = type opaque
%class.YaQSslError = type { %class.QSslError }
%class.QSslError = type { %class.QScopedPointer.6 }
%class.QScopedPointer.6 = type { %class.QSslErrorPrivate* }
%class.QSslErrorPrivate = type opaque
%class.YaQSslKey = type { %class.QSslKey }
%class.QSslKey = type { %class.QExplicitlySharedDataPointer.7 }
%class.QExplicitlySharedDataPointer.7 = type { %class.QSslKeyPrivate* }
%class.QSslKeyPrivate = type opaque
%class.YaQNetworkCookie = type { %class.QNetworkCookie }
%class.QNetworkCookie = type { %class.QSharedDataPointer.8 }
%class.QSharedDataPointer.8 = type { %class.QNetworkCookiePrivate* }
%class.QNetworkCookiePrivate = type opaque
%class.YaQNetworkCookieJar = type { %class.QNetworkCookieJar }
%class.QNetworkCookieJar = type { %class.QObject }
%class.YaQNetworkAccessManager = type { %class.QNetworkAccessManager }
%class.QNetworkAccessManager = type { %class.QObject }
%class.YaQNetworkRequest = type { %class.QNetworkRequest }
%class.QNetworkRequest = type { %class.QSharedDataPointer.9 }
%class.QSharedDataPointer.9 = type { %class.QNetworkRequestPrivate* }
%class.QNetworkRequestPrivate = type opaque
%class.YaQNetworkReply = type { %class.QNetworkReply }
%class.QNetworkReply = type { %class.QIODevice }
%class.YaQGuiApplication = type { %class.QGuiApplication }
%class.QGuiApplication = type { %class.QCoreApplication }
%class.YaQIcon = type { %class.QIcon }
%class.QIcon = type { %class.QIconPrivate* }
%class.QIconPrivate = type opaque
%class.YaQWidget = type { %class.QWidget }
%class.QWidget = type { %class.QObject, %class.QPaintDevice, %class.QWidgetData* }
%class.QWidgetData = type { i64, i32, %class.QFlags.10, i32, %class.QRect, %class.QPalette, %class.QFont, %class.QRect }
%class.QFlags.10 = type { i32 }
%class.QPalette = type { %class.QPalettePrivate*, %union.anon.11 }
%class.QPalettePrivate = type opaque
%union.anon.11 = type { %"struct.QPalette::Data" }
%"struct.QPalette::Data" = type { i32 }
%class.QFont = type { %class.QExplicitlySharedDataPointer.12, i32 }
%class.QExplicitlySharedDataPointer.12 = type { %class.QFontPrivate* }
%class.QFontPrivate = type opaque
%class.QRect = type { i32, i32, i32, i32 }
%class.YaQMainWindow = type { %class.QMainWindow }
%class.QMainWindow = type { %class.QWidget }
%struct.QTypedArrayData.13 = type { %struct.QArrayData }

@.str = private unnamed_addr constant [64 x i8] c"size == 0 || offset < 0 || size_t(offset) >= sizeof(QArrayData)\00", align 1
@.str1 = private unnamed_addr constant [36 x i8] c"/usr/include/qt/QtCore/qarraydata.h\00", align 1
@_ZN10QArrayData11shared_nullE = external global [2 x %struct.QArrayData]

; Function Attrs: nounwind uwtable
define void @_Z16__keep_jit_typesv() #0 {
  %v0 = alloca i8*, align 8
  store i8* null, i8** %v0, align 8
  %1 = load i8** %v0, align 8
  %2 = bitcast i8* %1 to %class.YaQAccessible*
  %3 = load i8** %v0, align 8
  %4 = bitcast i8* %3 to %class.QAccessible*
  %5 = load i8** %v0, align 8
  %6 = bitcast i8* %5 to %class.YaQAction*
  %7 = load i8** %v0, align 8
  %8 = bitcast i8* %7 to %class.QAction*
  %9 = load i8** %v0, align 8
  %10 = bitcast i8* %9 to %class.YaQActionGroup*
  %11 = load i8** %v0, align 8
  %12 = bitcast i8* %11 to %class.QActionGroup*
  %13 = load i8** %v0, align 8
  %14 = bitcast i8* %13 to %class.YaQObject*
  %15 = load i8** %v0, align 8
  %16 = bitcast i8* %15 to %class.QObject*
  %17 = load i8** %v0, align 8
  %18 = bitcast i8* %17 to %class.YaQString*
  %19 = load i8** %v0, align 8
  %20 = bitcast i8* %19 to %class.QString*
  %21 = load i8** %v0, align 8
  %22 = bitcast i8* %21 to %class.YaQBitArray*
  %23 = load i8** %v0, align 8
  %24 = bitcast i8* %23 to %class.QBitArray*
  %25 = load i8** %v0, align 8
  %26 = bitcast i8* %25 to %class.YaQBitmap*
  %27 = load i8** %v0, align 8
  %28 = bitcast i8* %27 to %class.QBitmap*
  %29 = load i8** %v0, align 8
  %30 = bitcast i8* %29 to %class.YaQBuffer*
  %31 = load i8** %v0, align 8
  %32 = bitcast i8* %31 to %class.QBuffer*
  %33 = load i8** %v0, align 8
  %34 = bitcast i8* %33 to %class.YaQBoxLayout*
  %35 = load i8** %v0, align 8
  %36 = bitcast i8* %35 to %class.QBoxLayout*
  %37 = load i8** %v0, align 8
  %38 = bitcast i8* %37 to %class.YaQByteArray*
  %39 = load i8** %v0, align 8
  %40 = bitcast i8* %39 to %class.QByteArray*
  %41 = load i8** %v0, align 8
  %42 = bitcast i8* %41 to %class.YaQThread*
  %43 = load i8** %v0, align 8
  %44 = bitcast i8* %43 to %class.QThread*
  %45 = load i8** %v0, align 8
  %46 = bitcast i8* %45 to %class.YaQCoreApplication*
  %47 = load i8** %v0, align 8
  %48 = bitcast i8* %47 to %class.QCoreApplication*
  %49 = load i8** %v0, align 8
  %50 = bitcast i8* %49 to %class.YaQChar*
  %51 = load i8** %v0, align 8
  %52 = bitcast i8* %51 to %class.QChar*
  %53 = load i8** %v0, align 8
  %54 = bitcast i8* %53 to %class.YaQStringList*
  %55 = load i8** %v0, align 8
  %56 = bitcast i8* %55 to %class.QStringList*
  %57 = load i8** %v0, align 8
  %58 = bitcast i8* %57 to %class.YaQTimer*
  %59 = load i8** %v0, align 8
  %60 = bitcast i8* %59 to %class.QTimer*
  %61 = load i8** %v0, align 8
  %62 = bitcast i8* %61 to %class.YaQDateTime*
  %63 = load i8** %v0, align 8
  %64 = bitcast i8* %63 to %class.QDateTime*
  %65 = load i8** %v0, align 8
  %66 = bitcast i8* %65 to %class.YaQUrl*
  %67 = load i8** %v0, align 8
  %68 = bitcast i8* %67 to %class.QUrl*
  %69 = load i8** %v0, align 8
  %70 = bitcast i8* %69 to %class.YaQFile*
  %71 = load i8** %v0, align 8
  %72 = bitcast i8* %71 to %class.QFile*
  %73 = load i8** %v0, align 8
  %74 = bitcast i8* %73 to %class.YaQRegExp*
  %75 = load i8** %v0, align 8
  %76 = bitcast i8* %75 to %class.QRegExp*
  %77 = load i8** %v0, align 8
  %78 = bitcast i8* %77 to %class.YaQHostAddress*
  %79 = load i8** %v0, align 8
  %80 = bitcast i8* %79 to %class.QHostAddress*
  %81 = load i8** %v0, align 8
  %82 = bitcast i8* %81 to %class.YaQAbstractSocket*
  %83 = load i8** %v0, align 8
  %84 = bitcast i8* %83 to %class.QAbstractSocket*
  %85 = load i8** %v0, align 8
  %86 = bitcast i8* %85 to %class.YaQTcpSocket*
  %87 = load i8** %v0, align 8
  %88 = bitcast i8* %87 to %class.QTcpSocket*
  %89 = load i8** %v0, align 8
  %90 = bitcast i8* %89 to %class.YaQTcpServer*
  %91 = load i8** %v0, align 8
  %92 = bitcast i8* %91 to %class.QTcpServer*
  %93 = load i8** %v0, align 8
  %94 = bitcast i8* %93 to %class.YaQSslSocket*
  %95 = load i8** %v0, align 8
  %96 = bitcast i8* %95 to %class.QSslSocket*
  %97 = load i8** %v0, align 8
  %98 = bitcast i8* %97 to %class.YaQSslCipher*
  %99 = load i8** %v0, align 8
  %100 = bitcast i8* %99 to %class.QSslCipher*
  %101 = load i8** %v0, align 8
  %102 = bitcast i8* %101 to %class.YaQSslConfiguration*
  %103 = load i8** %v0, align 8
  %104 = bitcast i8* %103 to %class.QSslConfiguration*
  %105 = load i8** %v0, align 8
  %106 = bitcast i8* %105 to %class.YaQSslError*
  %107 = load i8** %v0, align 8
  %108 = bitcast i8* %107 to %class.QSslError*
  %109 = load i8** %v0, align 8
  %110 = bitcast i8* %109 to %class.YaQSslKey*
  %111 = load i8** %v0, align 8
  %112 = bitcast i8* %111 to %class.QSslKey*
  %113 = load i8** %v0, align 8
  %114 = bitcast i8* %113 to %class.YaQNetworkCookie*
  %115 = load i8** %v0, align 8
  %116 = bitcast i8* %115 to %class.QNetworkCookie*
  %117 = load i8** %v0, align 8
  %118 = bitcast i8* %117 to %class.YaQNetworkCookieJar*
  %119 = load i8** %v0, align 8
  %120 = bitcast i8* %119 to %class.QNetworkCookieJar*
  %121 = load i8** %v0, align 8
  %122 = bitcast i8* %121 to %class.YaQNetworkAccessManager*
  %123 = load i8** %v0, align 8
  %124 = bitcast i8* %123 to %class.QNetworkAccessManager*
  %125 = load i8** %v0, align 8
  %126 = bitcast i8* %125 to %class.YaQNetworkRequest*
  %127 = load i8** %v0, align 8
  %128 = bitcast i8* %127 to %class.QNetworkRequest*
  %129 = load i8** %v0, align 8
  %130 = bitcast i8* %129 to %class.YaQNetworkReply*
  %131 = load i8** %v0, align 8
  %132 = bitcast i8* %131 to %class.QNetworkReply*
  %133 = load i8** %v0, align 8
  %134 = bitcast i8* %133 to %class.YaQGuiApplication*
  %135 = load i8** %v0, align 8
  %136 = bitcast i8* %135 to %class.QGuiApplication*
  %137 = load i8** %v0, align 8
  %138 = bitcast i8* %137 to %class.YaQIcon*
  %139 = load i8** %v0, align 8
  %140 = bitcast i8* %139 to %class.QIcon*
  %141 = load i8** %v0, align 8
  %142 = bitcast i8* %141 to %class.YaQWidget*
  %143 = load i8** %v0, align 8
  %144 = bitcast i8* %143 to %class.QWidget*
  %145 = load i8** %v0, align 8
  %146 = bitcast i8* %145 to %class.YaQMainWindow*
  %147 = load i8** %v0, align 8
  %148 = bitcast i8* %147 to %class.QMainWindow*
  ret void
}

; Function Attrs: uwtable
define void @_Z31__keep_jit_cannot_gen_functionsv() #1 {
  %1 = alloca %struct.QTypedArrayData.0, align 8
  %2 = alloca %struct.QTypedArrayData, align 8
  %3 = alloca %struct.QTypedArrayData.13, align 8
  %4 = call %struct.QTypedArrayData* @_ZN15QTypedArrayDataItE10sharedNullEv()
  %5 = call %struct.QTypedArrayData.0* @_ZN15QTypedArrayDataIcE10sharedNullEv()
  %6 = bitcast %struct.QTypedArrayData.0* %1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %6, i8 0, i64 24, i32 8, i1 false)
  %7 = call i8* @_ZN15QTypedArrayDataIcE4dataEv(%struct.QTypedArrayData.0* %1)
  %8 = bitcast %struct.QTypedArrayData* %2 to i8*
  call void @llvm.memset.p0i8.i64(i8* %8, i8 0, i64 24, i32 8, i1 false)
  %9 = call i16* @_ZN15QTypedArrayDataItE4dataEv(%struct.QTypedArrayData* %2)
  %10 = bitcast %struct.QTypedArrayData.13* %3 to i8*
  call void @llvm.memset.p0i8.i64(i8* %10, i8 0, i64 24, i32 8, i1 false)
  %11 = call i32* @_ZN15QTypedArrayDataIiE4dataEv(%struct.QTypedArrayData.13* %3)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr %struct.QTypedArrayData* @_ZN15QTypedArrayDataItE10sharedNullEv() #1 align 2 {
  %1 = call %struct.QArrayData* @_ZN10QArrayData10sharedNullEv()
  %2 = bitcast %struct.QArrayData* %1 to %struct.QTypedArrayData*
  ret %struct.QTypedArrayData* %2
}

; Function Attrs: nounwind uwtable
define linkonce_odr %struct.QTypedArrayData.0* @_ZN15QTypedArrayDataIcE10sharedNullEv() #0 align 2 {
  %1 = call %struct.QArrayData* @_ZN10QArrayData10sharedNullEv()
  %2 = bitcast %struct.QArrayData* %1 to %struct.QTypedArrayData.0*
  ret %struct.QTypedArrayData.0* %2
}

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #2

; Function Attrs: uwtable
define linkonce_odr i8* @_ZN15QTypedArrayDataIcE4dataEv(%struct.QTypedArrayData.0* %this) #1 align 2 {
  %1 = alloca %struct.QTypedArrayData.0*, align 8
  store %struct.QTypedArrayData.0* %this, %struct.QTypedArrayData.0** %1, align 8
  %2 = load %struct.QTypedArrayData.0** %1
  %3 = bitcast %struct.QTypedArrayData.0* %2 to %struct.QArrayData*
  %4 = call i8* @_ZN10QArrayData4dataEv(%struct.QArrayData* %3)
  ret i8* %4
}

; Function Attrs: uwtable
define linkonce_odr i16* @_ZN15QTypedArrayDataItE4dataEv(%struct.QTypedArrayData* %this) #1 align 2 {
  %1 = alloca %struct.QTypedArrayData*, align 8
  store %struct.QTypedArrayData* %this, %struct.QTypedArrayData** %1, align 8
  %2 = load %struct.QTypedArrayData** %1
  %3 = bitcast %struct.QTypedArrayData* %2 to %struct.QArrayData*
  %4 = call i8* @_ZN10QArrayData4dataEv(%struct.QArrayData* %3)
  %5 = bitcast i8* %4 to i16*
  ret i16* %5
}

; Function Attrs: uwtable
define linkonce_odr i32* @_ZN15QTypedArrayDataIiE4dataEv(%struct.QTypedArrayData.13* %this) #1 align 2 {
  %1 = alloca %struct.QTypedArrayData.13*, align 8
  store %struct.QTypedArrayData.13* %this, %struct.QTypedArrayData.13** %1, align 8
  %2 = load %struct.QTypedArrayData.13** %1
  %3 = bitcast %struct.QTypedArrayData.13* %2 to %struct.QArrayData*
  %4 = call i8* @_ZN10QArrayData4dataEv(%struct.QArrayData* %3)
  %5 = bitcast i8* %4 to i32*
  ret i32* %5
}

; Function Attrs: uwtable
define linkonce_odr i8* @_ZN10QArrayData4dataEv(%struct.QArrayData* %this) #1 align 2 {
  %1 = alloca %struct.QArrayData*, align 8
  store %struct.QArrayData* %this, %struct.QArrayData** %1, align 8
  %2 = load %struct.QArrayData** %1
  %3 = getelementptr inbounds %struct.QArrayData* %2, i32 0, i32 1
  %4 = load i32* %3, align 4
  %5 = icmp eq i32 %4, 0
  br i1 %5, label %16, label %6

; <label>:6                                       ; preds = %0
  %7 = getelementptr inbounds %struct.QArrayData* %2, i32 0, i32 3
  %8 = load i64* %7, align 8
  %9 = icmp slt i64 %8, 0
  br i1 %9, label %16, label %10

; <label>:10                                      ; preds = %6
  %11 = getelementptr inbounds %struct.QArrayData* %2, i32 0, i32 3
  %12 = load i64* %11, align 8
  %13 = icmp uge i64 %12, 24
  br i1 %13, label %16, label %14

; <label>:14                                      ; preds = %10
  call void @_Z9qt_assertPKcS0_i(i8* getelementptr inbounds ([64 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([36 x i8]* @.str1, i32 0, i32 0), i32 62) #5
  unreachable
                                                  ; No predecessors!
  br label %17

; <label>:16                                      ; preds = %10, %6, %0
  call void @_Z7qt_noopv()
  br label %17

; <label>:17                                      ; preds = %16, %15
  %18 = bitcast %struct.QArrayData* %2 to i8*
  %19 = getelementptr inbounds %struct.QArrayData* %2, i32 0, i32 3
  %20 = load i64* %19, align 8
  %21 = getelementptr inbounds i8* %18, i64 %20
  ret i8* %21
}

; Function Attrs: noreturn nounwind
declare void @_Z9qt_assertPKcS0_i(i8*, i8*, i32) #3

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr void @_Z7qt_noopv() #4 {
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr %struct.QArrayData* @_ZN10QArrayData10sharedNullEv() #0 align 2 {
  ret %struct.QArrayData* getelementptr inbounds ([2 x %struct.QArrayData]* @_ZN10QArrayData11shared_nullE, i32 0, i32 0)
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noreturn nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { inlinehint nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { noreturn nounwind }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.0 (tags/RELEASE_350/final)"}
