#pragma once
#include <string>

struct MercadoPagoConfig {
    std::string accessToken;
    std::string baseUrl;
};

class MercadoPagoClient {
public:
    MercadoPagoClient(const MercadoPagoConfig& config);

    std::string CreatePaymentPreference(
        const std::string& title,
        const std::string& description,
        const std::string& price,
        const std::string& currency,
        const std::string& externalReference
    );

private:
    MercadoPagoConfig config_;
    std::string HttpPost(const std::string& url, const std::string& payload);
};