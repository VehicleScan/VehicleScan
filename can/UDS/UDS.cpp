#include "UDS.hpp"

UDS::UDS(MCP251x mcp251x) : can(mcp251x) {}

bool UDS:: sendSingleFrame(const UDS_Msg& msg,CAN_ID canID){
    std::array<uint8_t, 8> frame = {0};
    uint8_t len = 1 + msg.req_params_length; 
    
    if (len > 7) return false;
    
    frame[0] = static_cast<uint8_t>(PCI::SINGLE_FRAME) | len;
    frame[1] = static_cast<uint8_t>(msg.sid);
    for (size_t i = 0; i < msg.req_params_length && i < 6; i++) {
        frame[i + 2] = msg.ReqParams[i];
    }    
    return can.transmit(frame, len + 1,static_cast<uint32_t>(canID), false);
}

bool UDS::sendRequest(const UDS_Msg& request) {   
    if ((request.req_params_length + 2) <= 7) {
        return sendSingleFrame(request,CAN_ID::CLIENT_ID);
    } 
    else {
        return false;
    }
}

UDS::UDS_Msg UDS:: processReceivedFrame(const std::array<uint8_t, 8>& data, uint8_t length, uint32_t can_id) {
    uint8_t pci_byte = data[0];
    PCI pci_type = static_cast<PCI>(pci_byte & 0xF0);
    
    switch (pci_type) {
        case PCI::SINGLE_FRAME:
            return handleSingleFrame(data,length);
            break;
        case PCI::FIRST_FRAME:
            
            break;
        case PCI::CONSECUTIVE_FRAME:
            
            break;
        case PCI::FLOW_CONTROL:
            break;
    }
    return UDS::UDS_Msg(); 
}

UDS::UDS_Msg UDS :: handleSingleFrame(const std::array<uint8_t, 8>& data, uint8_t length) {
        uint8_t pci_len = data[0] & 0x0F;
        
        UDS_Msg msg;
        msg.sid = static_cast<SID>(data[1]);
        
        for (uint8_t i = 2; i < 2 + (pci_len - 1); i++) {
            if (i >= length) break;
            msg.ReqParams.push_back(data[i]);
        }
        msg.req_params_length = pci_len-1;
        processMessage(msg);
        return msg;
}

void UDS::processMessage(const UDS_Msg& msg){
    if (static_cast<uint8_t>(msg.sid) < 0x40) {
        if (request_callback_) {
            request_callback_(msg);
        }
    } 
    else {
        if (response_callback_) {
            response_callback_(msg);
        }
    }
}

bool UDS :: checkReceived(std::array<uint8_t, 8>& data, uint8_t length,CAN_ID can_id = CAN_ID::CLIENT_ID){
    uint32_t id = static_cast<uint32_t>(can_id);
    if(can.receive(data,length,id)){
        processReceivedFrame(data,length,id);
        return true;
    }
    return false;
}

void UDS :: sendPositiveResponse(SID requested_sid, const std::vector<uint8_t>& data ){
    UDS_Msg response;
    response.sid = static_cast<SID>(static_cast<uint8_t>(requested_sid) + 0x40);
    response.ReqParams = data;
    response.req_params_length = data.size();
    sendRequest(response);
}

void UDS :: sendNegativeResponse(SID requested_sid, NRC nrc_code){
    std::array<uint8_t, 8> frame = {0};
    frame[0] = static_cast<uint8_t>(PCI::SINGLE_FRAME) | 0x03; 
    frame[1] = static_cast<uint8_t>(SID::NEGATIVE_RESPONSE);
    frame[2] = static_cast<uint8_t>(requested_sid);
    frame[3] = static_cast<uint8_t>(nrc_code);
    can.transmit(frame, 4,static_cast<uint32_t>(CAN_ID::SERVER_ID), false);
}
