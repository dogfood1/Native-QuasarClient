#include "stdafx.h"
#include "wmi_srvc.h"

#ifdef USE_WMI

using namespace std;
using namespace quasar::tools;

wmi_srvc::wmi_srvc() :
	m_is_init(true) {
	if (FAILED(CoInitialize(nullptr))) {
		m_is_init = false;
	}
	CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
	m_can_query = initialize_wbem() && m_is_init;
}

wmi_srvc::~wmi_srvc() {
	if(m_is_init) {
		CoUninitialize();
	}
}

bool wmi_srvc::simple_query(const string projection, const string from,
 string &output) const {
	return simple_query(projection, from, "", output);
}

bool wmi_srvc::simple_query(const string projection, const string from,
 const string where, string &output) const {
	if(!m_can_query) {
		return false;
	}
	CComPtr<IEnumWbemClassObject> wbemEnum;
	string rawQueryStr("Select " + projection);
	rawQueryStr.append(" From " + from);
	
	if (!where.empty()) {
		rawQueryStr.append(" Where " + where);
	}
	CComBSTR query = rawQueryStr.c_str();

	if(FAILED(m_wbem_services->ExecQuery(CComBSTR("WQL"), query,
	 WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &wbemEnum))) {
		return false;
	}

	ULONG uObjectCount = 0;
	CComPtr<IWbemClassObject> pWmiObject;
	if(FAILED(wbemEnum->Next(WBEM_INFINITE, 1, &pWmiObject, &uObjectCount))) {
		return false;
	}

	CComVariant cvtVersion;
	//TODO: use projection
	if(FAILED(pWmiObject->Get(L"Caption", 0, &cvtVersion, 0, 0))) {
		return false;
	}

	output = CW2A(cvtVersion.bstrVal);
	return true;
}

bool wmi_srvc::initialize_wbem() {
	if(FAILED(m_wbem_locator.CoCreateInstance(CLSID_WbemLocator))) {
		return false;
	}
	if(FAILED(m_wbem_locator->ConnectServer(CComBSTR(L"root\\cimv2"), nullptr,
	 nullptr, nullptr, 0, nullptr, nullptr, &m_wbem_services))) {
		return false;
	}
	return true;
}

#endif