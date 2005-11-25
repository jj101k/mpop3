#include <dns_sd.h>
#include <stdlib.h>

void handle_resolve_reply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *fullname, const char *hosttarget, uint16_t port, uint16_t txtLen, const char *txtRecord, void *context ) {
	printf("Service %s resolves to %s:%i\n", fullname, hosttarget, port);
} 

void handle_browse_reply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *replyType, const char *replyDomain, void *context) {
	printf("Found service named \"%s.%s%s\"\n", serviceName, replyType, replyDomain);
	DNSServiceRef sdResolveRef;
	DNSServiceErrorType err=DNSServiceResolve(&sdResolveRef, 0, interfaceIndex, serviceName, replyType, replyDomain, handle_resolve_reply, NULL); 
	if(err==kDNSServiceErr_NoError) {
		DNSServiceProcessResult(sdResolveRef);
		DNSServiceRefDeallocate(sdResolveRef);
	}
}

int main() {
	DNSServiceRef sdRef;
	DNSServiceErrorType err=DNSServiceBrowse(&sdRef, 0, 0, "_pop3._tcp", NULL, handle_browse_reply, NULL); 

	if(err==kDNSServiceErr_NoError) {
		DNSServiceProcessResult(sdRef);
		DNSServiceRefDeallocate(sdRef);
	}
	return 0;
}
