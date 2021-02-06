#include <wuapi.h>
#include <iostream>
#include <ATLComTime.h>
#include <wuerror.h>


int PrintWindowsUpdate()
{
    HRESULT hr;
    hr = CoInitialize(nullptr);
    {
        //IUpdateSession���t�@�N�g���ɂȂ��Ă��āAIUpdateSearcher�𐶐�����
        CComPtr<IUpdateSession> iUpdate;

        //IUpdateSession�C���^�[�t�F�[�X���擾����
        hr = CoCreateInstance(CLSID_UpdateSession, nullptr, CLSCTX_INPROC_SERVER, IID_IUpdateSession, (LPVOID*)&iUpdate);
        if (FAILED(hr))
        {
            std::wcout << "CoCreateInstance failed with error: " << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
            return 1;
        }

        //IUpdateSearcher�𐶐�����
        CComPtr<IUpdateSearcher> pSearcher;
        hr = iUpdate->CreateUpdateSearcher(&pSearcher);
        if (FAILED(hr))
        {
            std::wcout << "CreateUpdateSearcher failed with error: " << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
            return 1;
        }

        std::wcout << L"Searching for updates ..." << std::endl;

        //������̎w��
        //IsInstalled=1�F���̃R���s���[�^�ɃC���X�g�[������Ă���X�V�v���O��������������
        //IsHidden=1   �F���̃R���s���[�^�Ŕ�\���ƂȂ��Ă���X�V�v���O��������������
        //IsPresent    �F���̃R���s���[�^�ɑ��݂���X�V�v���O��������������
        //���̑��̌�����͈ȉ����Q��
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
            std::wcout << L"�Ǘ��Ώۂ̃T�[�o�[���Â����߁A�X�V�v���O�����������ł��܂���ł����B" << std::endl;
            return 1;
        case WU_E_INVALID_CRITERIA:
            std::wcout << L"���������������ł��B" << std::endl;
            return 1;
        }

        CComPtr<IUpdateCollection> updateList;
        pSearchResults->get_Updates(&updateList);
        LONG updateSize;
        updateList->get_Count(&updateSize);

        if (updateSize == 0)
        {
            std::wcout << L"�X�V�v���O������������܂���ł����B" << std::endl;
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
    //CoUninitialize()�̌��CComPtr�̃f�X�g���N�^���Ă΂�Ȃ��悤�Ƀu���b�N��
    ::CoUninitialize();

    return 0;
}