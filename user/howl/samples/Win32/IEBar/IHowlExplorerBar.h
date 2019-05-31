
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0280 */
/* at Sun Mar 23 17:23:47 2003, 2004
 */
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __srctree_h__
#define __srctree_h__

/* Forward Declarations */ 

#ifndef __IHowlExplorerBar_FWD_DEFINED__
#define __IHowlExplorerBar_FWD_DEFINED__
typedef interface IHowlExplorerBar IHowlExplorerBar;
#endif 	/* __IHowlExplorerBar_FWD_DEFINED__ */


#ifndef __HowlExplorerBar_FWD_DEFINED__
#define __HowlExplorerBar_FWD_DEFINED__

#ifdef __cplusplus
typedef class HowlExplorerBar HowlExplorerBar;
#else
typedef struct HowlExplorerBar HowlExplorerBar;
#endif /* __cplusplus */

#endif 	/* __HowlExplorerBar_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"
{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IHowlExplorerBar_INTERFACE_DEFINED__
#define __IHowlExplorerBar_INTERFACE_DEFINED__

/* interface IHowlExplorerBar */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IHowlExplorerBar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0FFBCB04-B7FC-4AEA-BD5A-32A07DB93DD7")
    IHowlExplorerBar : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IHowlExplorerBarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IHowlExplorerBar __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IHowlExplorerBar __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IHowlExplorerBar __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IHowlExplorerBar __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IHowlExplorerBar __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IHowlExplorerBar __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IHowlExplorerBar __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } IHowlExplorerBarVtbl;

    interface IHowlExplorerBar
    {
        CONST_VTBL struct IHowlExplorerBarVtbl __RPC_FAR *lpVtbl;
    };

#ifdef COBJMACROS


#define IHowlExplorerBar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHowlExplorerBar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHowlExplorerBar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHowlExplorerBar_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IHowlExplorerBar_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IHowlExplorerBar_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IHowlExplorerBar_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IHowlExplorerBar_INTERFACE_DEFINED__ */



#ifndef __HOWL_EXPLORER_BAR_Lib_LIBRARY_DEFINED__
#define __HOWL_EXPLORER_BAR_Lib_LIBRARY_DEFINED__

/* library HOWL_EXPLORER_BAR_Lib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_HOWL_EXPLORER_BAR_Lib;

EXTERN_C const CLSID CLSID_HowlExplorerBar;

#ifdef __cplusplus

class DECLSPEC_UUID("3588080F-0056-4d28-8519-F0479C73D40A")


HowlExplorerBar;
#endif
#endif /* __HOWL_EXPLORER_BAR_Lib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
