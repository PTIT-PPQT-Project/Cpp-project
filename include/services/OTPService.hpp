// include/services/OTPService.hpp
#pragma once

#include <string>
// No model includes needed here directly unless User object itself is passed
// but usually only the secret key from User is needed.

class OTPService {
public:
    OTPService();

    // Generates a new OTP secret key (Base32 encoded string)
    std::string generateNewOtpSecretKey() const;

    // Generates the OTPAuth URI for QR code generation
    std::string generateOtpUri(const std::string& username, const std::string& secretKey) const;

    // Verifies the OTP code entered by the user against their secret key
    bool verifyOtp(const std::string& otpSecretKey, const std::string& userEnteredOtp) const;
};