#include "SystemCA.h"
#include "WebSocket.h"

#ifdef PLATFORM_WINDOWS

#include <stdio.h>
#include <iostream>
#include <tchar.h>

#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"

#include <windows.h>
#include <wincrypt.h>
#include <cryptuiapi.h>

#define UI UI_ST
#include <openssl/ssl.h>
//#include <openssl/x509.h>
#undef UI

#include "Windows/HideWindowsPlatformTypes.h"


#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

int add_system_ca_bundle(void *vssl_ctx)
{
	SSL_CTX *ssl_ctx = (SSL_CTX*)vssl_ctx;
	X509_STORE *store = SSL_CTX_get_cert_store(ssl_ctx);
	HCERTSTORE hStore;
	PCCERT_CONTEXT pContext = NULL;
	X509 *x509;
	
	hStore = CertOpenSystemStore(NULL, L"ROOT");

	if (!hStore)
		return 1;

	while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != 0)
	{
		//uncomment the line below if you want to see the certificates as pop ups
		//CryptUIDlgViewContext(CERT_STORE_CERTIFICATE_CONTEXT, pContext,   NULL, NULL, 0, NULL);

		const unsigned char *pbCertEncoded = pContext->pbCertEncoded;
		x509 = d2i_X509(NULL, &pbCertEncoded, pContext->cbCertEncoded);
		if (x509)
		{
			int i = X509_STORE_add_cert(store, x509);


			if (i == 1)
			{
				char *cname = X509_NAME_oneline(X509_get_issuer_name(x509), 0, 0);
				FString subject(cname);
				OPENSSL_free(cname);
				UE_LOG(WebSocket, Log, TEXT("Added Certificate: %s"), *subject);
			}

			X509_free(x509);
		}
	}
	CertCloseStore(hStore, 0);
	return 0;
}

#endif // _WIN32