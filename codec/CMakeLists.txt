set(CSR ${CMAKE_CURRENT_SOURCE_DIR})
set(OCGCORE_ENCODER_HDRS
	${CSR}/encoder_impl_begin.hpp
	${CSR}/encoder_impl_end.hpp
	${CSR}/encoder_information_impl.hpp
	${CSR}/encoder_request_impl.hpp
	${CSR}/encoder_spec_information_impl.hpp
)

target_sources(ygopen PRIVATE
	${CSR}/buffer.hpp
	${CSR}/core_message.hpp
	${CSR}/msg_encoder.cpp
	${CSR}/msgs_map.hpp
)

target_sources(ygopen PUBLIC
	${CSR}/msg_codec.hpp
)
unset(CSR)
