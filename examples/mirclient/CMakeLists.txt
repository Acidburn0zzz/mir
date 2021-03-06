set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -fno-strict-aliasing -Wextra")

include_directories(
  ${PROJECT_SOURCE_DIR}/include/client
)

add_library(mirdraw STATIC graphics_utils.cpp graphics.h)
target_include_directories(mirdraw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

mir_add_wrapped_executable(mir_demo_client_basic basic.c)
target_link_libraries     (mir_demo_client_basic mirclient ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

mir_add_wrapped_executable(mir_demo_client_release_at_exit release_at_exit.c)
target_link_libraries     (mir_demo_client_release_at_exit mirclient)

mir_add_wrapped_executable(mir_demo_client_multiwin multiwin.c)
target_link_libraries     (mir_demo_client_multiwin mirclient)

mir_add_wrapped_executable(mir_demo_client_fingerpaint fingerpaint.c)
target_link_libraries     (mir_demo_client_fingerpaint mirclient)

mir_add_wrapped_executable(mir_demo_client_progressbar progressbar.c)
target_link_libraries     (mir_demo_client_progressbar mirclient)

mir_add_wrapped_executable(mir_demo_client_flicker flicker.c)
target_link_libraries     (mir_demo_client_flicker mirclient ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

mir_add_wrapped_executable(mir_demo_client_prompt_session prompt_session.c)
target_link_libraries     (mir_demo_client_prompt_session mirclient)

if (MIR_ENABLE_TESTS)
  add_executable(mir_symbol_test
      uses_non_client_symbol.cpp
  )
  target_link_libraries(mir_symbol_test
      mirclient
  )

  # Regression tests for LP: #1415321
  # To see reasons for failure:  ctest -V -R Clients-only-use-client-symbols
  mir_add_test(NAME Clients-only-use-client-symbols
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMAND /bin/sh -c "!(objdump -T mir_demo_client* | grep *UND* | grep mir | grep -v MIR_CLIENT)"
  )

  # Test our testcase...
  mir_add_test(NAME Client-using-symbols-from-mircommon-fails-check
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMAND /bin/sh -c "(objdump -T mir_symbol_test | grep *UND* | grep mir | grep -v MIR_CLIENT)"
  )

  # We can't use this test with clang as that generates spurious NEEDED
  if (NOT "${CMAKE_CXX_COMPILER}" MATCHES "clang")
    # To see reasons for failure:  ctest -V -R Clients-only-use-client-ABI
    mir_add_test(NAME Clients-only-use-client-ABI
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
      COMMAND /bin/sh -c "!(objdump -p mir_demo_client*  | grep NEEDED | grep libmir | grep -v libmirclient)"
    )
    # Again, test the testcase...
    mir_add_test(NAME Client-using-ABI-from-mircommon-fails-check
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
      COMMAND /bin/sh -c "(objdump -p mir_symbol_test | grep NEEDED | grep libmir | grep -v libmirclient)"
    )
  endif ()
endif ()

mir_add_wrapped_executable(mir_demo_client_prerendered_frames prerendered_frames.c)
target_link_libraries     (mir_demo_client_prerendered_frames mirclient m)

mir_add_wrapped_executable(mir_demo_client_chain_jumping_buffers chain_jumping_buffers.c)
target_link_libraries     (mir_demo_client_chain_jumping_buffers mirclient)

include_directories(${MIR_GENERATED_INCLUDE_DIRECTORIES})

mir_add_wrapped_executable(mir_demo_client_wayland wayland_client.c)
target_link_libraries     (mir_demo_client_wayland ${WAYLAND_CLIENT_LDFLAGS} ${WAYLAND_CLIENT_LIBRARIES})

mir_add_wrapped_executable(mir_demo_client_screencast screencast.cpp)
target_link_libraries     (mir_demo_client_screencast mirclient)


# The remaining examples are dependent on Mir EGL being supported.
if (MIR_EGL_SUPPORTED)

  add_library(eglapp STATIC
    eglapp.c
    client_helpers.cpp
  )

  target_include_directories(eglapp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(eglapp
    mirclient
    ${EGL_LIBRARIES}
    ${GLESv2_LIBRARIES}
  )

  mir_add_wrapped_executable(mir_demo_client_animated_cursor animated_cursor_demo_client.c)
  target_link_libraries     (mir_demo_client_animated_cursor eglapp)

  mir_add_wrapped_executable(mir_demo_client_eglflash eglflash.c)
  target_link_libraries     (mir_demo_client_eglflash eglapp)

  mir_add_wrapped_executable(mir_demo_client_egltriangle egltriangle.c)
  target_link_libraries     (mir_demo_client_egltriangle eglapp)

  mir_add_wrapped_executable(mir_demo_client_target target.c)
  target_link_libraries     (mir_demo_client_target eglapp)

  mir_add_wrapped_executable(mir_demo_client_pointer_confinement pointer_confinement.c)
  target_link_libraries     (mir_demo_client_pointer_confinement eglapp)

  mir_add_wrapped_executable(mir_demo_client_camera camera.c)
  target_link_libraries     (mir_demo_client_camera eglapp)

  mir_add_wrapped_executable(mir_demo_client_eglcounter eglcounter.cpp)
  target_link_libraries     (mir_demo_client_eglcounter eglapp)

  mir_add_wrapped_executable(mir_demo_client_eglplasma eglplasma.c)
  target_link_libraries     (mir_demo_client_eglplasma eglapp)

  mir_add_wrapped_executable(mir_demo_client_cursors cursors_demo_client.c)
  target_link_libraries     (mir_demo_client_cursors eglapp)

  mir_add_wrapped_executable(mir_demo_client_eglstateswitcher eglstateswitcher.c)
  target_link_libraries     (mir_demo_client_eglstateswitcher eglapp)

  mir_add_wrapped_executable(mir_demo_client_tooltip tooltip.c)
  target_link_libraries     (mir_demo_client_tooltip eglapp)

  mir_add_wrapped_executable(mir_demo_client_touch_validator client_touch_validator.cpp)
  target_link_libraries     (mir_demo_client_touch_validator eglapp)

  mir_add_wrapped_executable(mir_demo_client_display_config demo_client_display_config.c)
  target_link_libraries     (mir_demo_client_display_config eglapp)

  mir_add_wrapped_executable(mir_demo_client_eglsquare eglsquare.cpp)
  target_link_libraries     (mir_demo_client_eglsquare eglapp)

  if ("${CMAKE_CXX_COMPILER}" MATCHES "clang")
    target_link_libraries   (mir_demo_client_eglsquare atomic)
  endif()

  mir_add_wrapped_executable(mir_demo_client_input_shapes input_shapes.cpp)
  target_link_libraries     (mir_demo_client_input_shapes eglapp)

  mir_add_wrapped_executable(mir_demo_client_scroll scroll.cpp)
  target_link_libraries     (mir_demo_client_scroll eglapp mirdraw ${GLESv2_LIBRARIES})

  mir_add_wrapped_executable(mir_demo_client_multistream multi_stream.cpp)
  target_link_libraries     (mir_demo_client_multistream mirclient eglapp ${Boost_LIBRARIES})

  mir_add_wrapped_executable(mir_demo_client_render_surface render_surface.cpp)
  target_link_libraries     (mir_demo_client_render_surface mirclient eglapp)

endif() # MIR_EGL_SUPPORTED
