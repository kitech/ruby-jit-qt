; ModuleID = 'metalize/jit_types.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.QArrayData = type { %"class.QtPrivate::RefCount", i32, i32, i64 }
%"class.QtPrivate::RefCount" = type { %class.QBasicAtomicInteger }
%class.QBasicAtomicInteger = type { i32 }
%class.YaQAccessible = type { i8 }
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

@_ZN10QArrayData11shared_nullE = external global [2 x %struct.QArrayData]

; Function Attrs: nounwind uwtable
define void @_Z16__keep_jit_typesv() #0 {
  %v0 = alloca i8*, align 8
  store i8* null, i8** %v0, align 8
  %1 = load i8** %v0, align 8
  %2 = bitcast i8* %1 to %class.YaQAccessible*
  %3 = load i8** %v0, align 8
  %4 = bitcast i8* %3 to %class.YaQAction*
  %5 = load i8** %v0, align 8
  %6 = bitcast i8* %5 to %class.YaQActionGroup*
  %7 = load i8** %v0, align 8
  %8 = bitcast i8* %7 to %class.YaQObject*
  %9 = load i8** %v0, align 8
  %10 = bitcast i8* %9 to %class.YaQString*
  %11 = load i8** %v0, align 8
  %12 = bitcast i8* %11 to %class.YaQBitArray*
  %13 = load i8** %v0, align 8
  %14 = bitcast i8* %13 to %class.YaQBitmap*
  %15 = load i8** %v0, align 8
  %16 = bitcast i8* %15 to %class.YaQBuffer*
  %17 = load i8** %v0, align 8
  %18 = bitcast i8* %17 to %class.YaQBoxLayout*
  %19 = load i8** %v0, align 8
  %20 = bitcast i8* %19 to %class.YaQByteArray*
  %21 = load i8** %v0, align 8
  %22 = bitcast i8* %21 to %class.YaQThread*
  %23 = load i8** %v0, align 8
  %24 = bitcast i8* %23 to %class.YaQCoreApplication*
  %25 = load i8** %v0, align 8
  %26 = bitcast i8* %25 to %class.YaQChar*
  %27 = load i8** %v0, align 8
  %28 = bitcast i8* %27 to %class.YaQStringList*
  %29 = load i8** %v0, align 8
  %30 = bitcast i8* %29 to %class.YaQTimer*
  %31 = load i8** %v0, align 8
  %32 = bitcast i8* %31 to %class.YaQDateTime*
  %33 = load i8** %v0, align 8
  %34 = bitcast i8* %33 to %class.YaQUrl*
  %35 = load i8** %v0, align 8
  %36 = bitcast i8* %35 to %class.YaQFile*
  %37 = load i8** %v0, align 8
  %38 = bitcast i8* %37 to %class.YaQRegExp*
  %39 = load i8** %v0, align 8
  %40 = bitcast i8* %39 to %class.YaQHostAddress*
  %41 = load i8** %v0, align 8
  %42 = bitcast i8* %41 to %class.YaQAbstractSocket*
  %43 = load i8** %v0, align 8
  %44 = bitcast i8* %43 to %class.YaQTcpSocket*
  %45 = load i8** %v0, align 8
  %46 = bitcast i8* %45 to %class.YaQTcpServer*
  %47 = load i8** %v0, align 8
  %48 = bitcast i8* %47 to %class.YaQSslSocket*
  %49 = load i8** %v0, align 8
  %50 = bitcast i8* %49 to %class.YaQSslCipher*
  %51 = load i8** %v0, align 8
  %52 = bitcast i8* %51 to %class.YaQSslConfiguration*
  %53 = load i8** %v0, align 8
  %54 = bitcast i8* %53 to %class.YaQSslError*
  %55 = load i8** %v0, align 8
  %56 = bitcast i8* %55 to %class.YaQSslKey*
  %57 = load i8** %v0, align 8
  %58 = bitcast i8* %57 to %class.YaQNetworkCookie*
  %59 = load i8** %v0, align 8
  %60 = bitcast i8* %59 to %class.YaQNetworkCookieJar*
  %61 = load i8** %v0, align 8
  %62 = bitcast i8* %61 to %class.YaQNetworkAccessManager*
  %63 = load i8** %v0, align 8
  %64 = bitcast i8* %63 to %class.YaQNetworkRequest*
  %65 = load i8** %v0, align 8
  %66 = bitcast i8* %65 to %class.YaQNetworkReply*
  %67 = load i8** %v0, align 8
  %68 = bitcast i8* %67 to %class.YaQGuiApplication*
  %69 = load i8** %v0, align 8
  %70 = bitcast i8* %69 to %class.YaQIcon*
  %71 = load i8** %v0, align 8
  %72 = bitcast i8* %71 to %class.YaQWidget*
  %73 = load i8** %v0, align 8
  %74 = bitcast i8* %73 to %class.YaQMainWindow*
  ret void
}

; Function Attrs: uwtable
define void @_Z31__keep_jit_cannot_gen_functionsv() #1 {
  %1 = call %struct.QTypedArrayData* @_ZN15QTypedArrayDataItE10sharedNullEv()
  ret void
}

; Function Attrs: uwtable
define linkonce_odr %struct.QTypedArrayData* @_ZN15QTypedArrayDataItE10sharedNullEv() #1 align 2 {
  %1 = call %struct.QArrayData* @_ZN10QArrayData10sharedNullEv()
  %2 = bitcast %struct.QArrayData* %1 to %struct.QTypedArrayData*
  ret %struct.QTypedArrayData* %2
}

; Function Attrs: nounwind uwtable
define linkonce_odr %struct.QArrayData* @_ZN10QArrayData10sharedNullEv() #0 align 2 {
  ret %struct.QArrayData* getelementptr inbounds ([2 x %struct.QArrayData]* @_ZN10QArrayData11shared_nullE, i32 0, i32 0)
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.0 (tags/RELEASE_350/final)"}
