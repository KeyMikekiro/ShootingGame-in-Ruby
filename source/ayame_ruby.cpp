/*
###################################
#
# Ayame/Ruby 0.0.2
#
###################################
*/

#include "ruby.h"
#include "VoiceElement.h"
#include "VoiceElementAyame.h"
#include "VoiceElementMidi.h"

static VALUE cAyame;
static VALUE eAyameError;  /* 例外                 */

struct AyameData {
    CVoiceElement* pVoiceElement;
    __int64 start_tick;
};

static int     g_isPerformanceCounter = 0;         /* パフォーマンスカウンタがあったら１ */
static __int64 g_OneSecondCount       = 0;         /* 一秒間にカウンタが数える数         */

static void InitSync( void );
static __int64 GetSystemCounter( void );

#define AYAME_GET_STRUCT( v ) ((struct AyameData *)DATA_PTR( v ))

/* 内部参照用リスト */
static struct AyameList {
    void **pointer;
    int allocate_size;
    int count;
} g_AyameList;

static bool g_AyameShutdownFlag = false;


static void AddAyameList( struct AyameData *ad )
{
    if( g_AyameList.allocate_size <= g_AyameList.count )
    {
        g_AyameList.allocate_size = g_AyameList.allocate_size * 3 / 2; /* 1.5倍にする */
        g_AyameList.pointer = (void**)realloc( g_AyameList.pointer, sizeof( void* ) * g_AyameList.allocate_size );
    }

    g_AyameList.pointer[g_AyameList.count] = ad;
    g_AyameList.count++;
}

static void DeleteAyameList( struct AyameData *ad )
{
    int i;

    for( i = 0; i < g_AyameList.count; i++ )
    {
        if( g_AyameList.pointer[i] == ad )
        {
            break;
        }
    }
    if( i == g_AyameList.count )
    {
        rb_raise( eAyameError, "内部エラー - DeleteAyameList" );
    }

    i++;
    for( ; i < g_AyameList.count; i++ )
    {
        g_AyameList.pointer[i - 1] = g_AyameList.pointer[i];
    }

    g_AyameList.count--;
}


static VALUE Ayame_dispose( VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement != NULL )
    {
        if( !g_AyameShutdownFlag )
        {
            ad->pVoiceElement->Release();
        }
        delete ad->pVoiceElement;
        ad->pVoiceElement = NULL;
        DeleteAyameList( ad );
    }
    return self;
}

static void Ayame_release( struct AyameData *ad )
{
    if( ad->pVoiceElement != NULL )
    {
        if( !g_AyameShutdownFlag )
        {
            ad->pVoiceElement->Release();
        }
        delete ad->pVoiceElement;
        ad->pVoiceElement = NULL;
        DeleteAyameList( ad );
    }
    free( ad );
}

static VALUE Ayame_allocate( VALUE klass )
{
    struct AyameData *ad = (struct AyameData *)malloc( sizeof( struct AyameData ) );
    ad->pVoiceElement = NULL;
    return Data_Wrap_Struct( klass, NULL, Ayame_release, ad );
}

static VALUE Ayame_initialize( VALUE self, VALUE vfilename )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );

    Check_Type(vfilename, T_STRING);
    if( strstr( RSTRING_PTR( vfilename ), ".mid" ) != NULL )
    {
        //midi
        ad->pVoiceElement = new CVoiceElementMidi;
        if( !ad->pVoiceElement->Load(RSTRING_PTR( vfilename ), 0, 0, 0) ) rb_raise( eAyameError, "%sのロードに失敗しました", vfilename );
        ad->pVoiceElement->SetVolume(100);
    }
    else
    {
        //ayame
        ad->pVoiceElement = new CVoiceElementAyame;
        if( !ad->pVoiceElement->Load(RSTRING_PTR( vfilename ), AYAME_LOADFLAG_GLOBAL | AYAME_LOADFLAG_STREAM, 0, 0) ) rb_raise( eAyameError, "%sのロードに失敗しました", vfilename );
        ad->pVoiceElement->SetVolume(100);
    }

    AddAyameList( ad );

    return self;
}

static VALUE Ayame_load_from_memory( VALUE klass, VALUE vdata )
{
    struct AyameData *ad;
    VALUE vad;

    Check_Type(vdata, T_STRING);
    vad = Ayame_allocate( klass );
    ad = AYAME_GET_STRUCT( vad );
    ad->pVoiceElement = new CVoiceElementAyame;
    if( !ad->pVoiceElement->Load((char*)RSTRING_PTR( vdata ), RSTRING_LEN( vdata ), AYAME_LOADFLAG_GLOBAL | AYAME_LOADFLAG_STATIC) ) rb_raise( eAyameError, "ロードに失敗しました" );
    ad->pVoiceElement->SetVolume(100);

    AddAyameList( ad );

    return vad;
}

static VALUE Ayame_play( int argc, VALUE *argv, VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );

    if( argc < 1 || argc > 2 ) rb_raise( rb_eArgError, "wrong number of arguments (%d for %d..%d)", argc, 1, 2 );

    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    ad->pVoiceElement->Play( NUM2INT( argv[0] ) - 1, 100, argc < 2 || argv[1] == Qnil ? 0 : NUM2INT( argv[1] ) * 1000 );
    ad->start_tick = GetSystemCounter();
    return self;
}

static VALUE Ayame_stop( int argc, VALUE *argv, VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );

    if( argc < 0 || argc > 1 ) rb_raise( rb_eArgError, "wrong number of arguments (%d for %d..%d)", argc, 0, 1 );

    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    ad->pVoiceElement->Stop(argc == 0 || argv[0] == Qnil ? 0.0 : NUM2INT( argv[0] ) * 1000 );
    return self;
}

static VALUE Ayame_set_volume( int argc, VALUE *argv, VALUE self )
//( VALUE self, VALUE vvolume, VALUE vtick )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );

    if( argc < 1 || argc > 2 ) rb_raise( rb_eArgError, "wrong number of arguments (%d for %d..%d)", argc, 1, 2 );

    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    ad->pVoiceElement->SetVolume( NUM2INT( argv[0] ), argc < 2 || argv[1] == Qnil ? 0 : NUM2INT( argv[1] ) * 1000.0 );
    return self;
}

static VALUE Ayame_get_volume( VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    return INT2NUM(ad->pVoiceElement->GetVolume());
}

static VALUE Ayame_set_pan( int argc, VALUE *argv, VALUE self )
//( VALUE self, VALUE vpan, VALUE vtick )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );

    if( argc < 1 || argc > 2 ) rb_raise( rb_eArgError, "wrong number of arguments (%d for %d..%d)", argc, 1, 2 );

    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    ad->pVoiceElement->SetPan( NUM2INT( argv[0] ),  argc < 2 || argv[1] == Qnil ? 0 : NUM2INT( argv[1] ) * 1000.0 );
    return self;
}

static VALUE Ayame_get_pan( VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    return INT2NUM(ad->pVoiceElement->GetPan    ());
}

static VALUE Ayame_pause( VALUE self, VALUE vtick )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    ad->pVoiceElement->Pause( true, NUM2INT( vtick ) * 1000.0 );
    return self;
}

static VALUE Ayame_resume( VALUE self, VALUE vtick )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    ad->pVoiceElement->Pause( false, NUM2INT( vtick ) * 1000.0 );
    return self;
}

static VALUE Ayame_prefetch( VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    ad->pVoiceElement->Prefetch();
    return self;
}

static VALUE Ayame_predecode( VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    ad->pVoiceElement->Predecode();
    return self;
}

static VALUE Ayame_IsFade( VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    if( ad->pVoiceElement->IsFade() )
    {
        return Qtrue;
    }
    else
    {
        return Qfalse;
    }
}

static VALUE Ayame_IsPlay( VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    if( ad->pVoiceElement->IsPlaying() )
    {
        return Qtrue;
    }
    else
    {
        return Qfalse;
    }
}

static VALUE Ayame_IsFinish( VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    if( ad->pVoiceElement->IsFinished() )
    {
        return Qtrue;
    }
    else
    {
        return Qfalse;
    }
}

static VALUE Ayame_IsPause( VALUE self )
{
    struct AyameData *ad = AYAME_GET_STRUCT( self );
    if( ad->pVoiceElement == NULL ) rb_raise( eAyameError, "disposed Ayame object" );
    if( ad->pVoiceElement->IsPause() )
    {
        return Qtrue;
    }
    else
    {
        return Qfalse;
    }
}

static VALUE Ayame_update( VALUE klass )
{
    int i;

    for( i = 0; i < g_AyameList.count; i++ )
    {
        struct AyameData *ad = (struct AyameData *)g_AyameList.pointer[i];
        __int64 tick = GetSystemCounter();
        ad->pVoiceElement->Run((double)(tick - ad->start_tick) / (double)g_OneSecondCount * 1000.0);
        ad->start_tick = tick;
    }
    return Qnil;
}

/*--------------------------------------------------------------------
  （内部関数）フレーム調整用カウンタ値取得
 ---------------------------------------------------------------------*/
static __int64 GetSystemCounter( void )
{
    __int64 time;

    if( g_isPerformanceCounter == 1 )
    {
        QueryPerformanceCounter( (LARGE_INTEGER *)&time );
        return time;
    }
    else
    {
        return timeGetTime();
    }
}

/*--------------------------------------------------------------------
  （内部関数）フレーム調整初期化
 ---------------------------------------------------------------------*/
static void InitSync( void )
{
    timeBeginPeriod( 1 );

    /* パフォーマンスカウンタの秒間カウント値取得 */
    if( QueryPerformanceFrequency( (LARGE_INTEGER *)&g_OneSecondCount ) )
    {
        /* パフォーマンスカウンタがある場合 */
        g_isPerformanceCounter = 1;
    }
    else
    {
        /* パフォーマンスカウンタが無い場合 */
        g_isPerformanceCounter = 0;
        g_OneSecondCount = 1000;
    }
}

static void Ayame_shutdown( VALUE obj )
{
    CVoiceElementAyame::m_sAyameMgr.Release();
    CoUninitialize();
    g_AyameShutdownFlag = true;
}


/*
***************************************************************
*
*         Global functions
*
***************************************************************/

LRESULT CALLBACK DummyWndProc( HWND hWnd,UINT msg,UINT wParam,LONG lParam )
{
    return DefWindowProc( hWnd, msg, wParam, lParam );
}


extern "C" {
    void Init_ayame();
}

void Init_ayame()
{
    HRESULT hr;
    HWND hWnd;
    WNDCLASSEX wcex;
    RECT rect;
    HINSTANCE hInstance;

    CoInitialize(NULL);

    /* インスタンスハンドル取得 */
    hInstance = (HINSTANCE)GetModuleHandle( NULL );

    /* ウインドウ・クラスの登録 */
    wcex.cbSize        = sizeof( WNDCLASSEX );
    wcex.style         = 0;
    wcex.lpfnWndProc   = DefWindowProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = 0;
    wcex.hIconSm       = 0;
    wcex.hCursor       = 0;
    wcex.hbrBackground = 0;
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = "Ayame";

    if( !RegisterClassEx( &wcex ) )
    {
        rb_raise( eAyameError, "ウィンドウの初期化に失敗しました - RegusterClassEx" );
    }

    hWnd = CreateWindow("Ayame", "", WS_POPUP, 0, 0, 0, 0, 0, NULL, hInstance, NULL);

    /* 例外定義 */
    eAyameError = rb_define_class( "AyameError", rb_eRuntimeError );

    if ( !CVoiceElementAyame::m_sAyameMgr.LoadDLL( hWnd, "Ayame.dll", 0 ) ) rb_raise( eAyameError, "Ayame.dllがロードできませんでした" );

    /* Ayameクラス定義 */
    cAyame = rb_define_class ( "Ayame", rb_cObject );

    /* Ayameクラスにメソッド登録*/
    rb_define_private_method( cAyame, "initialize", (VALUE(*)(...))Ayame_initialize, 1 );
    rb_define_singleton_method( cAyame, "load_from_memory", (VALUE(*)(...))Ayame_load_from_memory, 1 );
    rb_define_method( cAyame, "play", (VALUE(*)(...))Ayame_play, -1 );
    rb_define_method( cAyame, "stop", (VALUE(*)(...))Ayame_stop, -1 );
    rb_define_method( cAyame, "get_volume", (VALUE(*)(...))Ayame_get_volume, 0 );
    rb_define_method( cAyame, "set_volume", (VALUE(*)(...))Ayame_set_volume, -1 );
    rb_define_method( cAyame, "get_pan", (VALUE(*)(...))Ayame_get_pan, 0 );
    rb_define_method( cAyame, "set_pan", (VALUE(*)(...))Ayame_set_pan, -1 );
    rb_define_method( cAyame, "dispose", (VALUE(*)(...))Ayame_dispose, 0 );
    rb_define_method( cAyame, "pause", (VALUE(*)(...))Ayame_pause, 1 );
    rb_define_method( cAyame, "resume", (VALUE(*)(...))Ayame_resume, 1 );
    rb_define_method( cAyame, "prefetch", (VALUE(*)(...))Ayame_prefetch, 0 );
    rb_define_method( cAyame, "predecode", (VALUE(*)(...))Ayame_predecode, 0 );

    rb_define_method( cAyame, "fading?", (VALUE(*)(...))Ayame_IsFade, 0 );
    rb_define_method( cAyame, "playing?", (VALUE(*)(...))Ayame_IsPlay, 0 );
    rb_define_method( cAyame, "finished?", (VALUE(*)(...))Ayame_IsFinish, 0 );
    rb_define_method( cAyame, "pausing?", (VALUE(*)(...))Ayame_IsPause, 0 );

    rb_define_singleton_method( cAyame, "update", (VALUE(*)(...))Ayame_update, 0 );

    /* Ayameオブジェクトを生成した時にinitializeの前に呼ばれるメモリ割り当て関数登録 */
    rb_define_alloc_func( cAyame, Ayame_allocate );

    /* 終了時に実行する関数 */
    rb_set_end_proc( (void(*)(VALUE))Ayame_shutdown, Qnil );

    InitSync();

    /* Ayameオブジェクトの内部参照リスト */
    g_AyameList.pointer = (void**)malloc( sizeof(void*) * 16 );
    g_AyameList.count = 0;
    g_AyameList.allocate_size = 16;
}
