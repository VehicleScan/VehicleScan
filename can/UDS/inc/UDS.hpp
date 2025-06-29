#ifndef UDS_HPP
#define UDS_HPP
#include "MCP251x.hpp"
#include <cstdint>
#include <array>
#include <vector>
#include <queue>

class UDS{
    public:
    enum class PCI:uint8_t{
        SINGLE_FRAME = 0x00,
        FIRST_FRAME = 0x10,
        CONSECUTIVE_FRAME = 0x20,
        FLOW_CONTROL = 0x30
    };

    enum class SID:uint8_t{
        DIAG_SESSION_CTRL  = 0x10,
        ECU_RESET          = 0x11,
        READ_DATA          = 0x22,
        WRITE_DATA         = 0x2E,
        ROUTINE_CTRL       = 0x31,
        REQUEST_DOWNLOAD   = 0x34,
        TRANSFER_DATA      = 0x36,
        REQUEST_TRANS_EXIT = 0x37,
        NEGATIVE_RESPONSE  = 0x7F
    };

    enum class NRC{
        POSITIVE_RESPONSE  = 0x00,
        SERVICE_NOT_SUPP   = 0x11,
        SUBFUNC_NOT_SUPP   = 0x12,
        INCORRECT_MSG_LEN  = 0x13,
        CONDITIONS_NOT_MET = 0x22,
        REQUEST_SEQ_ERROR  = 0x24,
        REQUEST_OUT_OF_RNG = 0x31,
        SECURITY_ACCESS_DN = 0x33,
        INVALID_KEY        = 0x35,
        EXCEED_NUM_ATTMPT  = 0x36,
        TIME_DELAY_EXCEED  = 0x37,
        UPLOAD_DOWNLOAD    = 0x70,
        TRANSFER_SUSPENDED = 0x71,
        GEN_PROGRAM_FAIL   = 0x72
    };

    enum class CAN_ID:uint16_t{
        CLIENT_ID = 0x7E0,
        SERVER_ID = 0x7E8,
    };

    struct UDS_Msg{   
        SID sid;
        PCI pci;
        std::vector<uint8_t> ReqParams;
        uint8_t req_params_length = 0;
        UDS_Msg() : sid(SID::NEGATIVE_RESPONSE), pci(PCI::SINGLE_FRAME), req_params_length(0) {}
    };
    
    explicit UDS(MCP251x mcp251x);

    bool sendRequest(const UDS_Msg& request);
    
    void registerOnRequestReceivedCB(std::function<void(const UDS_Msg&)> callback);

    void registerOnResponseReceivedCB(std::function<void(const UDS_Msg&)> callback);

    void sendNegativeResponse(SID requested_sid, NRC nrc_code);

    void sendPositiveResponse(SID requested_sid, const std::vector<uint8_t>& data = {});

    bool checkReceived(std::array<uint8_t, 8>& data, uint8_t length,CAN_ID can_id);

    private:
    MCP251x can;
    std::function<void(const UDS_Msg&)> request_callback_;
    std::function<void(const UDS_Msg&)> response_callback_;

    UDS_Msg processReceivedFrame(const std::array<uint8_t, 8>& data, uint8_t length, uint32_t can_id);
    bool sendSingleFrame(const UDS_Msg& msg,CAN_ID canId);
    void processMessage(const UDS_Msg& msg);
    UDS_Msg handleSingleFrame(const std::array<uint8_t, 8>& data, uint8_t length);
    std::vector<uint8_t> sync_Request(UDS_Msg& request);
};
#endif // UDS_HPP