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
					canUdsServiceHandler \
					DiagnosticApp \
					CustomCarSystemUIBottomRoundedRRO

BOARD_SEPOLICY_DIRS += 	vendor/VehicleScan/can_uds/client_test/se_policy \
						vendor/VehicleScan/can_setup/se_linux

