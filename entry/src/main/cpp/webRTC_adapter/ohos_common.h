#ifndef OH_WEB_RTC_OHOS_COMMON_H
#define OH_WEB_RTC_OHOS_COMMON_H

class CommonInfo {
public:
    static CommonInfo& GetInstance(){
        static CommonInfo ohosCommonInfo;
        return ohosCommonInfo;
    };
    CommonInfo(){};
    ~CommonInfo(){};
    char currentPath[512];
};

#endif //OH_WEB_RTC_OHOS_COMMON_H




