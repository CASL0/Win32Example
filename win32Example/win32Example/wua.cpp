#include <wuapi.h>
#include <iostream>
#include <ATLComTime.h>
#include <wuerror.h>


int PrintWindowsUpdate()
{
    HRESULT hr;
    hr = CoInitialize(nullptr);
    {
        //IUpdateSessionがファクトリになっていて、IUpdateSearcherを生成する
        CComPtr<IUpdateSession> iUpdate;

        //IUpdateSessionインターフェースを取得する
        hr = CoCreateInstance(CLSID_UpdateSession, nullptr, CLSCTX_INPROC_SERVER, IID_IUpdateSession, (LPVOID*)&iUpdate);
        if (FAILED(hr))
        {
            std::wcout << "CoCreateInstance failed with error: " << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
            return 1;
        }

        //IUpdateSearcherを生成する
        CComPtr<IUpdateSearcher> pSearcher;
        hr = iUpdate->CreateUpdateSearcher(&pSearcher);
        if (FAILED(hr))
        {
            std::wcout << "CreateUpdateSearcher failed with error: " << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
            return 1;
        }

        std::wcout << L"Searching for updates ..." << std::endl;

        //検索句の指定
        //IsInstalled=1：このコンピュータにインストールされている更新プログラムを検索する
        //IsHidden=1   ：このコンピュータで非表示となっている更新プログラムを検索する
        //IsPresent    ：このコンピュータに存在する更新プログラムを検索する
        //その他の検索句は以下を参照
        //https://docs.microsoft.com/en-us/windows/win32/api/wuapi/nf-wuapi-iupdatesearcher-search
        BSTR criteria = SysAllocString(L"IsInstalled=1 or IsHidden=1 or IsPresent=1");
        CComPtr<ISearchResult> pSearchResults;
        hr = pSearcher->Search(criteria, &pSearchResults);
        SysFreeString(criteria);

        switch (hr)
        {
        case S_OK:
            std::wcout << L"List of applicable items on the machine:" << std::endl;
            break;
        case WU_E_LEGACYSERVER:
            std::wcout << L"管理対象のサーバーが古いため、更新プログラムを検索できませんでした。" << std::endl;
            return 1;
        case WU_E_INVALID_CRITERIA:
            std::wcout << L"検索条件が無効です。" << std::endl;
            return 1;
        }

        CComPtr<IUpdateCollection> updateList;
        pSearchResults->get_Updates(&updateList);
        LONG updateSize;
        updateList->get_Count(&updateSize);

        if (updateSize == 0)
        {
            std::wcout << L"更新プログラムが見つかりませんでした。" << std::endl;
        }

        IUpdate* updateItem;
        BSTR updateName;
        DATE retDate;

        for (LONG i = 0; i < updateSize; i++)
        {
            updateList->get_Item(i, &updateItem);
            updateItem->get_Title(&updateName);
            updateItem->get_LastDeploymentChangeTime(&retDate);
            COleDateTime odt;
            odt.m_dt = retDate;
            std::wcout << "[" << i + 1 << "]" << " - " << updateName << "  Release Date " << (LPCTSTR)odt.Format(_T("%A, %B %d, %Y")) << std::endl;
        }
    }
    //CoUninitialize()の後にCComPtrのデストラクタが呼ばれないようにブロック化
    ::CoUninitialize();

    return 0;
}