PRODUCT_PACKAGES += udsServer \
 					udsClient \
					libcantp \
					canTpTest \
					canTpSender \
					canTpReceiver \
					libcanudstpserver \
					libcanudstpclient \
					canUdsServerTest \
					canUdsClientTest \
					can_setup \
					canUdsServiceHandler

BOARD_SEPOLICY_DIRS += 	vendor/VehicleScan/can_uds/client_test/se_policy