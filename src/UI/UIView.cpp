#include "UI/UIView.h"

#include "interaction/CreateMovieAction.h"
#include "interaction/HelperFunctions.h"

#include "vrapp/VRVolumeApp.h"

#include <glm/gtc/type_ptr.hpp>

#include "loader/VRDataLoader.h"

#include "UIHelpers/stb_image.h"
#include "common/common.h"
#include "../include/interaction/Simulation.h"

#include <limits>
#include <iomanip>

#include <imgui_stdlib.h>
#include <thread>

UIView::UIView(VRVolumeApp &controllerApp) : m_controller_app(controllerApp), m_multiplier(1.0f), m_threshold(0.0f),
                                             m_z_scale(0.16f), m_scale{1.0f}, m_slices(256), m_dynamic_slices(false), m_renderVolume(true), m_selectedTrnFnc(0),
                                             m_animated(false), m_ui_frame_controller(0.0f), m_menu_handler(nullptr), m_initialized(false), m_use_transferfunction(false),
                                             m_clip_max(1.0), m_clip_min(0.0), m_clip_ypr(0.0), m_clip_pos(0.0), m_useCustomClipPlane(false), m_rendermethod(1), m_renderchannel(0),
                                             m_trnfnc_table_selection(-1), m_transfer_function_options_window(false), m_save_transfer_function_open(false), m_trnfnct_counter(1), m_file_dialog_open(false),
                                             m_file_load_trnsf(false), m_file_dialog_save_dir(false), m_save_session_dialog_open(false), m_current_save_modal(SAVE_NONE),
                                             m_current_load_modal(LOAD_NONE), m_file_extension_filter(".txt"), m_non_trns_functions_selected_modal(false),
                                             m_ui_background(false), m_column_selection_state(0), m_compute_new_histogram(true), m_histogram_point_1(0.0),
                                             m_histogram_point_2(1.1), m_stopped(false), m_color_map_directory("colormaps"), m_show_menu(true), m_camera_poi_table_selection(0),
                                             m_camera_name_window_open(false), m_camera_button_action(BUTTON_ACTION::NONE), m_num_animation_frames(0), m_animation_step(1.0f),
                                             m_camera_animation_duration_open(false), m_show_clock(false), m_clock_width(250), m_clock_height(200),
                                             m_time_info(""), m_day_info(""), m_time_frame_edited(false), m_show_movie_saved_pop_up(false)
{
  m_ocean_color_maps_names = {"algae.png", "amp.png", "balance.png", "curl.png", "deep.png", "delta.png", "dense.png",
                              "diff.png", "gray.png", "haline.png", "ice.png", "matter.png", "oxy.png", "phase.png", "rain.png", "solar.png", "speed.png", "tarn.png", "tempo.png",
                              "thermal.png", "topo.png", "turbid.png"};
  m_histogram_quantiles[0] = 0.05;
  m_histogram_quantiles[1] = 0.95;
}

UIView::~UIView()
{
  delete m_menu_handler;
}

void UIView::draw_ui_callback()
{
  if (!m_initialized)
  {
    return;
  }

  if (m_controller_app.is_show_menu())
  {
    m_ui_background = m_menu_handler->windowIsActive() ? true : false;

    int flags = ImGuiWindowFlags_NoResize;
    if (!m_ui_background)
    {
      flags |= ImGuiWindowFlags_NoBackground;
    }
    ImGui::SetNextWindowSize(ImVec2(544, 798));
    ImGui::Begin("Volumeviewer", NULL, flags);
    ImGui::BeginTabBar("##tabs");
    if (ImGui::BeginTabItem("General"))
    {
      if (ImGui::Button("load file", ImVec2(ImGui::GetWindowSize().x * 0.5f - 1.5 * ImGui::GetStyle().ItemSpacing.x, 0.0f)))
      {
        m_file_dialog_open = true;
        m_file_extension_filter = ".txt";
      }

      ImGui::SameLine();
      if (ImGui::Button("Clear all", ImVec2(ImGui::GetWindowSize().x * 0.5f - 1.5 * ImGui::GetStyle().ItemSpacing.x, 0.0f)))
      {
        m_controller_app.clear_data();
      }

      ImGui::SliderFloat("alpha multiplier", &m_multiplier, 0.0f, 1.0f, "%.3f");
      ImGui::SliderFloat("threshold", &m_threshold, 0.0f, 1.0f, "%.3f");
      ImGui::SliderFloat("scale", &m_scale, 0.001f, 5.0f, "%.3f");
      ImGui::SliderFloat("z - scale", &m_z_scale, 0.001f, 5.0f, "%.3f");
      ImGui::SliderInt("Slices", &m_slices, 10, 1024, "%d");
      ImGui::Checkbox("automatic slice adjustment", &m_dynamic_slices);
      ImGui::Checkbox("Show Clock", &m_show_clock);

      ImGui::SameLine(ImGui::GetWindowSize().x * 0.5f, 0);
      ImGui::Text("FPS = %f", m_controller_app.get_fps());
      const char *items[] = {"sliced", "raycast"};
      ImGui::Combo("RenderMethod", &m_rendermethod, items, IM_ARRAYSIZE(items));

      const char *items_channel[] = {"based on data", "red", "green", "blue", "alpha", "rgba", "rgba with alpha as max rgb"};
      ImGui::Combo("Render Channel", &m_renderchannel, items_channel, IM_ARRAYSIZE(items_channel));

      ImGui::Checkbox("Render Volume data", &m_renderVolume);

      int numVolumes = m_controller_app.get_num_volumes();

      if (numVolumes > 0)
      {
        if (ImGui::SmallButton("Save Session"))
        {
          m_save_session_dialog_open = true;
        }
        ImGui::SameLine();
      }

      if (ImGui::SmallButton("Load Session"))
      {
        m_file_load_trnsf = true;
        m_current_load_modal = LOAD_MODAL::LOAD_SESSION;
        m_file_extension_filter = ".usr";
      }

      if (numVolumes > 0)
      {
        if (ImGui::SmallButton("Add Function"))
        {
          tfn_widget.push_back(TransferFunctionWidget());
          tfn_widget_multi.push_back(TransferFunctionMultiChannelWidget());
          int index = m_selected_volume_TrFn.size();
          m_selected_volume_TrFn.push_back(std::vector<bool>(numVolumes));
          for (int i = 0; i < numVolumes; i++)
          {
            m_selected_volume_TrFn[index][i] = false;
          }

          add_transfer_function();
          if (m_tfns.size() == 1) m_trnfnc_table_selection = 0;
          
        };
        ImGui::SameLine();
        if (ImGui::SmallButton("Remove Function"))
        {

          if (m_tfns.size() == 1)
          {
            // there should be one by default
            tfn_widget.clear();
            tfn_widget_multi.clear();
            m_tfns.clear();
            tfn_widget.push_back(TransferFunctionWidget());
            tfn_widget_multi.push_back(TransferFunctionMultiChannelWidget());
            add_transfer_function();
            m_trnfnc_table_selection = 0;
          }
          else if (m_tfns.size() > 1 && m_trnfnc_table_selection >= 0)
          {
            tfn_widget.erase(tfn_widget.begin() + m_trnfnc_table_selection);
            ;
            tfn_widget_multi.erase(tfn_widget_multi.begin() + m_trnfnc_table_selection);
            m_tfns.erase(m_tfns.begin() + m_trnfnc_table_selection);
            if (m_trnfnc_table_selection != 0)
            {
              m_trnfnc_table_selection = m_trnfnc_table_selection - 1;
            }
          }
        };

        if (ImGui::SmallButton("Clear All Functions"))
        {
          tfn_widget.clear();
          tfn_widget_multi.clear();
          m_tfns.clear();
          tfn_widget.push_back(TransferFunctionWidget());
          tfn_widget_multi.push_back(TransferFunctionMultiChannelWidget());
          MyTransFerFunctions trfntc;
          char label[32];
          sprintf(label, "TF%d", (int)tfn_widget.size());
          trfntc.ID = tfn_widget.size();
          trfntc.Name = label;
          for (int i = 0; i < numVolumes; i++)
          {
            trfntc.volumes.push_back(false);
          }
          m_tfns.push_back(trfntc);
          m_trnfnc_table_selection = 0;
        };

        ImGui::SameLine();
        if (ImGui::SmallButton("Save Functions"))
        {
          m_save_transfer_function_open = true;
          m_non_trns_functions_selected_modal = !m_use_transferfunction;
        };

        ImGui::SameLine();
        if (ImGui::SmallButton("Load Functions"))
        {

          m_file_load_trnsf = true;
          m_current_load_modal = LOAD_MODAL::LOAD_TRFR_FNC;
          m_file_extension_filter = ".fnc";
        };

        int selection = 0;

        ImGui::BeginTable("##Transfer Function Editor", numVolumes + 1, ImGuiTableFlags_Borders);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);

        for (int column = 0; column < numVolumes; column++)
        {
          ImGui::TableSetupColumn(m_dataLabels[column].c_str());
        }
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        for (int column = 1; column < numVolumes + 1; column++)
        {
          ImGui::TableSetColumnIndex(column);
          const char *column_name = ImGui::TableGetColumnName(column);
          ImGui::PushID(column);
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
          ImGui::Checkbox("##checkall", &m_column_selected[column - 1]);
          ImGui::SameLine();
          ImGui::PopStyleVar();
          ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
          ImGui::TableHeader(column_name);
          ImGui::PopID();
        }

        compute_new_histogram_view();

        /* check state of check boxes */
        int colum_select_check_sum = 0;
        for (int column_select = 1; column_select <= numVolumes; column_select++)
        {

          if (m_column_selected[column_select - 1])
          {
            colum_select_check_sum |= column_select;
          }
        }

        if (colum_select_check_sum != m_column_selection_state)
        {
          m_compute_new_histogram = true;
          m_column_selection_state = colum_select_check_sum;
        }
        else
        {
          m_compute_new_histogram = false;
        }

        for (int row = 0; row < m_tfns.size(); row++)
        {
          ImGui::TableNextRow();
          for (int col = 0; col < numVolumes + 1; col++)
          {
            ImGui::TableSetColumnIndex(col);

            if (col == 0)
            {

              ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_AllowDoubleClick;
              bool item_is_selected = (row == m_trnfnc_table_selection) ? true : false;
              if (ImGui::Selectable(m_tfns[row].Name.c_str(), item_is_selected, selectable_flags))
              {
                m_trnfnc_table_selection = row;
                if (ImGui::IsMouseDoubleClicked(0))
                {
                  m_copy_trnfnct_name = m_tfns[m_trnfnc_table_selection].Name;
                  float q_min = 0;
                  float q_max = 0;
                  tfn_widget[m_trnfnc_table_selection].get_quantiles(q_min, q_max);
                  m_histogram_quantiles[0] = q_min;
                  m_histogram_quantiles[1] = q_max;
                  m_transfer_function_options_window = true;
                }
              }
            }
            else
            {
              char buf[32];
              sprintf(buf, "##On%d%d", col, row);
              bool b = m_tfns[row].volumes[col - 1];
              ImGui::Checkbox(buf, &b);
              m_tfns[row].volumes[col - 1] = b;
            }
          }
        }

        ImGui::EndTable();
        if (m_save_session_dialog_open)
        {
          m_current_save_modal = SAVE_SESSION;
          std::string save_user_session_window_id = "Save User Session";

          std::string extension = ".usr";
          auto save_funtion = std::bind(&UIView::save_user_session, this, std::placeholders::_1);
          open_save_modal_dialog(save_user_session_window_id, m_save_session_dialog_open, save_funtion, extension);
        }

        if (m_save_transfer_function_open)
        {
          m_current_save_modal = SAVE_TRANSFER_FUNCTION;
          std::string save_Trnf_window_id = "Save Transfer Functions";
          if (m_use_transferfunction)
          {
            std::string extension = ".fnc";
            auto save_funtion = std::bind(&UIView::save_transfer_functions, this, std::placeholders::_1);
            open_save_modal_dialog(save_Trnf_window_id, m_save_transfer_function_open, save_funtion, extension);
          }
          else
          {
            ImGui::OpenPopup("No Functions Selected");
            ImGui::SetNextWindowSize(ImVec2(150, 70), ImGuiCond_FirstUseEver);
            if (ImGui::BeginPopupModal("No Functions Selected", &m_non_trns_functions_selected_modal))
            {
              ImGui::Text("Transfer functions are not enabled.");
              ImGui::Text("Please, check the 'Use Transfer function' option.");
              if (ImGui::Button("Close"))
              {
                m_non_trns_functions_selected_modal = false;
                m_save_transfer_function_open = false;
                ImGui::CloseCurrentPopup();
              }
              ImGui::EndPopup();
            }
          }
        }

        if (m_transfer_function_options_window)
        {
          ImGui::OpenPopup("Transfer Function Options");
          ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
          if (ImGui::BeginPopupModal("Transfer Function Options", &m_transfer_function_options_window))
          {
            ImGui::Text("Change Name");
            ImGui::InputText("##text1", &m_copy_trnfnct_name);
            ImGui::IsItemActive();

            ImGui::Separator();

            ImGui::Text("Quantiles");
            ImGui::SliderFloat2("##Quantiles", m_histogram_quantiles, 0.05f, 0.95f);

            if (ImGui::Button("Adjust to Histogram"))
            {
              adjust_transfer_function_to_histogram();
              tfn_widget[m_trnfnc_table_selection].alpha_control_pts.clear();
              tfn_widget[m_trnfnc_table_selection].alpha_control_pts.push_back(vec2f(0.0, 0.0));
              tfn_widget[m_trnfnc_table_selection].alpha_control_pts.push_back(m_histogram_point_1);
              tfn_widget[m_trnfnc_table_selection].alpha_control_pts.push_back(m_histogram_point_2);
              tfn_widget[m_trnfnc_table_selection].alpha_control_pts.push_back(vec2f(1.0, 1.0));
              tfn_widget[m_trnfnc_table_selection].set_quantiles(m_histogram_quantiles[0], m_histogram_quantiles[1]);
              ImGui::OpenPopup("Confirmation");
              ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
            }

            bool unused_open = true;
            if (ImGui::BeginPopupModal("Confirmation", &unused_open))
            {
              ImGui::Text("Change Applied");
              if (ImGui::Button("Close"))
              {
                ImGui::CloseCurrentPopup();
              }
              ImGui::EndPopup();
            }

            if (ImGui::Button("Ok"))
            {
              m_tfns[m_trnfnc_table_selection].Name = m_copy_trnfnct_name;
              m_transfer_function_options_window = false;
              m_current_save_modal = SAVE_MODAL::SAVE_NONE;
              ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
              m_transfer_function_options_window = false;
              m_current_save_modal = SAVE_MODAL::SAVE_NONE;
              ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
          }
        }

        ImGui::Checkbox("use transferfunction", &m_use_transferfunction);
        if (m_use_transferfunction)
        {

          bool is_multi_channel = m_controller_app.data_is_multi_channel();
          if (is_multi_channel)
          {
            for (int i = 0; i < 3; i++)
            {
              if (m_animated)
              {
                /*
                  TODO:
                  
                  Fix frame by frame animation

                  unsigned int active_volume = floor(m_frame);
                  unsigned int active_volume2 = ceil(m_frame);
                  double alpha = m_frame - active_volume;
                  if (active_volume < m_volumes[m_selectedVolume].size() && active_volume2 < m_volumes[m_selectedVolume].size())
                  {
                    tfn_widget_multi[m_selectedVolume].setBlendedHistogram(
                      m_volumes[m_selectedVolume][active_volume]->getTransferfunction(i),
                      m_volumes[m_selectedVolume][active_volume2]->getTransferfunction(i), alpha, i);
                  }*/
              }
              
            }
            m_controller_app.set_multi_transfer(true);
            tfn_widget_multi[m_trnfnc_table_selection].draw_ui();
          }
          else
          {
            if (m_animated)
            {
              /*
              TODO:
                  
              The base code to fix frame by frame animation

              unsigned int active_volume = floor(m_frame);
                unsigned int active_volume2 = ceil(m_frame);
                double alpha = m_frame - active_volume;
                tfn_widget[m_selectedVolume].setMinMax(m_volumes[m_selectedVolume][active_volume]->getMin() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMin() * (1.0 - alpha),
                  m_volumes[m_selectedVolume][active_volume]->getMax() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMax() * (1.0 - alpha));
                if (active_volume < m_volumes[m_selectedVolume].size() && active_volume2 < m_volumes[m_selectedVolume].size())
                  tfn_widget[m_selectedVolume].setBlendedHistogram(m_volumes[m_selectedVolume][active_volume]->getTransferfunction(0), m_volumes[m_selectedVolume][active_volume2]->getTransferfunction(0), alpha);*/
            }

            m_controller_app.set_multi_transfer(false);

            m_histogram.draw_histogram();
            tfn_widget[m_trnfnc_table_selection].draw_ui();
            
          }
        }
      }

      if (m_animated)
      {
        ImGui::Text("Timestep");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-100 - ImGui::GetStyle().ItemSpacing.x);
        float frame_tmp = m_controller_app.get_current_frame() + 1;
        // controls animated multi datasets
        ImGui::SliderFloat("##Timestep", &frame_tmp, 1, m_num_animation_frames);
        m_ui_frame_controller = (frame_tmp)-1;
        m_controller_app.set_frame(m_ui_frame_controller);

        std::string text = m_stopped ? "Play" : "Stop";
        if (ImGui::Button(text.c_str(), ImVec2(50, 0)))
        {
          m_stopped = !m_stopped;
        }
        ImGui::SameLine();
        int value = int(m_animation_step * 100.0 + .5);
        std::string speed_text = ">>X " + std::to_string(value / 100);
        if (ImGui::Button(speed_text.c_str(), ImVec2(80, 0)))
        {
          m_animation_step += 0.5;
          if (m_animation_step > 4.0f)
          {
            m_animation_step = 1.0f;
          }
          m_controller_app.set_volume_animation_scale_factor(m_animation_step);
        }

#if (!defined(__APPLE__))
        std::string movie_button_label = m_controller_app.get_movie_state_label();
        if (movie_button_label == "Write Movie")
        {
          if (ImGui::Button(movie_button_label.c_str()))
          {
            m_controller_app.run_movie(false);
          }
        }
        else
        {
          if (ImGui::Button(movie_button_label.c_str()))
          {
            m_controller_app.stop_movie();
          }
        }

#endif
      }
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Clipping"))
    {
      ImGui::Text("Axis aligned clip");
      glm::vec2 bound = {m_clip_min.x * 100, m_clip_max.x * 100};
      ImGui::DragFloatRange2("X", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
      m_clip_min.x = bound.x / 100;
      m_clip_max.x = bound.y / 100;

      bound = {m_clip_min.y * 100, m_clip_max.y * 100};
      ImGui::DragFloatRange2("Y", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
      m_clip_min.y = bound.x / 100;
      m_clip_max.y = bound.y / 100;

      bound = {m_clip_min.z * 100, m_clip_max.z * 100};
      ImGui::DragFloatRange2("Z", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
      m_clip_min.z = bound.x / 100;
      m_clip_max.z = bound.y / 100;

      if (ImGui::Button("Reset"))
      {
        m_clip_min = glm::vec3(0.0f);
        m_clip_max = glm::vec3(1.0f);
      }

      ImGui::Checkbox("Custom Clipping plane", &m_useCustomClipPlane);
      if (m_useCustomClipPlane)
      {
        ImGui::SliderAngle("Pitch", &m_clip_ypr.y, -90, 90);
        ImGui::SliderAngle("Roll", &m_clip_ypr.z, -180, 180);

        ImGui::SliderFloat("Position X", &m_clip_pos.x, -0.5, 0.5);
        ImGui::SliderFloat("Position y", &m_clip_pos.y, -0.5, 0.5);
        ImGui::SliderFloat("Position z", &m_clip_pos.z, -0.5, 0.5);
        if (ImGui::Button("Reset##Reset2"))
        {
          m_clip_ypr = glm::vec3(0.0f);
          m_clip_pos = glm::vec3(0.0f);
        }
      }
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Animation"))
    {

      ImGui::Text("Time Frames");

      if (ImGui::Button("Add"))
      {
        m_camera_name_window_open = true;
        m_camera_button_action = BUTTON_ACTION::ADD;
      }
      ImGui::SameLine();
      if (ImGui::Button("Remove"))
      {
        if (m_controller_app.get_simulation().get_simulation_states().size() > 0)
        {
          m_controller_app.get_simulation().remove_simulation_state(m_simulation_state_selection);
          if (m_simulation_state_selection - 1 < 0)
          {
            m_simulation_state_selection = 0;
          }
          else
          {
            m_simulation_state_selection--;
            SimulationState next_sim_state = m_controller_app.get_simulation().get_simulation_state_at(m_simulation_state_selection);
            m_controller_app.get_trackball_camera().set_current_poi(next_sim_state.poi);
          }
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("Reset Camera"))
      {
        m_controller_app.get_trackball_camera().reset_camera();
      }
      ImGui::BeginTable("##Camera Point of interest (POI) Editor", 1, ImGuiTableFlags_Borders);
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);

      for (int row = 0; row < m_controller_app.get_simulation().get_simulation_states().size(); ++row)
      {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        bool item_is_selected = (row == m_simulation_state_selection) ? true : false;
        SimulationState sim_state = m_controller_app.get_simulation().get_simulation_state_at(row);
        if (ImGui::Selectable(sim_state.poi.label.c_str(), item_is_selected, ImGuiSelectableFlags_AllowDoubleClick))
        {
          m_simulation_state_selection = row;
          m_controller_app.get_trackball_camera().set_current_poi(sim_state.poi);
          m_clip_min = sim_state.min_clip;
          m_clip_max = sim_state.max_clip;
          if (ImGui::IsMouseDoubleClicked(0))
          {
            m_copy_camera_name = sim_state.poi.label;
            m_camera_name_window_open = true;
            m_camera_button_action = BUTTON_ACTION::EDIT;
          }
        }
      }
      ImGui::EndTable();

      if (m_controller_app.get_simulation().get_simulation_states().size() > 0)
      {
        SimulationState &sim_state = m_controller_app.get_simulation().get_simulation_state_at(m_simulation_state_selection);
        std::string edit_text = "Edit Time Frame " + sim_state.poi.label;
        ImGui::Text(edit_text.c_str());

        ImGui::Text("Axis aligned clip");
        glm::vec2 bound = {m_clip_min.x * 100, m_clip_max.x * 100};
        ImGui::DragFloatRange2("X", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
        m_clip_min.x = bound.x / 100;
        m_clip_max.x = bound.y / 100;

        ImGui::SameLine();
        if (ImGui::Button("Reset##XAXIS"))
        {
          m_clip_min.x = 0.0f;
          m_clip_max.x = 1.0f;
        }

        bound = {m_clip_min.y * 100, m_clip_max.y * 100};
        ImGui::DragFloatRange2("Y", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
        m_clip_min.y = bound.x / 100;
        m_clip_max.y = bound.y / 100;
        ImGui::SameLine();
        if (ImGui::Button("Reset##YAXIS"))
        {
          m_clip_min.y = 0.0f;
          m_clip_max.y = 1.0f;
        }

        bound = {m_clip_min.z * 100, m_clip_max.z * 100};
        ImGui::DragFloatRange2("Z", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
        m_clip_min.z = bound.x / 100;
        m_clip_max.z = bound.y / 100;
        ImGui::SameLine();
        if (ImGui::Button("Reset##ZAXIS"))
        {
          m_clip_min.z = 0.0f;
          m_clip_max.z = 1.0f;
        }

        if (ImGui::Button("Save Changes##EDITTIMEFRAME"))
        {
          sim_state.min_clip = m_clip_min;
          sim_state.max_clip = m_clip_max;
          sim_state.poi = m_controller_app.get_trackball_camera().get_current_poi();
          m_time_frame_edited = true;
        }
      }

      // Simulation animator
      std::string is_animation_playing = m_controller_app.get_simulation().get_camera_animation_state();

      if (ImGui::Button(is_animation_playing.c_str()))
      {

        m_controller_app.get_simulation().set_animation_state();
      }

      ImGui::Text("Duration (seconds)");

      std::stringstream text;
      text << std::fixed << std::setprecision(1) << m_controller_app.get_simulation().get_animation_duration();
      std::string str_animation_duration = text.str();

      if (ImGui::Button(str_animation_duration.c_str()))
      {
        m_camera_animation_duration_open = true;
        m_string_animation_duration = str_animation_duration;
      }

#if (!defined(__APPLE__))
      std::string movie_button_label = m_controller_app.get_movie_state_label();
      if (movie_button_label == "Write Movie")
      {
        if (ImGui::Button(movie_button_label.c_str()))
        {
          m_controller_app.get_simulation().set_animation_state();
          m_controller_app.run_movie(true);
        }
      }
      else
      {
        if (ImGui::Button(movie_button_label.c_str()))
        {
          m_controller_app.stop_movie();
        }
      }
#endif

      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();

    if (m_time_frame_edited)
    {
      ImGui::OpenPopup("Time frame Changed##successchange");
      ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
      if (ImGui::BeginPopupModal("Time frame Changed##successchange", &m_time_frame_edited))
      {
        ImGui::Text("Changes saved");
        if (ImGui::Button("Ok"))
        {
          m_time_frame_edited = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }

    if (m_camera_name_window_open)
    {
      std::string modal_window_name;

      switch (m_camera_button_action)
      {
      case ADD:
        modal_window_name = "Add Simulation State";
        break;
      case EDIT:
        modal_window_name = "Edit Simulation Name";
        break;
      default:
        break;
      }

      ImGui::OpenPopup(modal_window_name.c_str());
      ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
      if (ImGui::BeginPopupModal(modal_window_name.c_str(), &m_camera_name_window_open))
      {

        ImGui::Text("Camera Name");
        ImGui::InputText("##textcameraname", &m_copy_camera_name);
        if (ImGui::Button("Ok"))
        {

          if (m_camera_button_action == ADD)
          {
            SimulationState sim_state;
            sim_state.poi = m_controller_app.get_trackball_camera().get_current_poi();
            sim_state.poi.label = m_copy_camera_name;
            sim_state.max_clip = m_clip_max;
            sim_state.min_clip = m_clip_min;
            m_controller_app.get_simulation().add_simulation_state(sim_state);

            m_copy_camera_name.clear();
            m_simulation_state_selection = m_controller_app.get_simulation().get_simulation_states().size() - 1;
          }
          else if (m_camera_button_action == EDIT)
          {
            m_controller_app.get_simulation().get_simulation_state_at(m_camera_poi_table_selection).poi.label = m_copy_camera_name;
          }
          m_camera_name_window_open = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
          m_camera_name_window_open = false;
          m_copy_camera_name.clear();
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }

    if (m_camera_animation_duration_open)
    {
      ImGui::OpenPopup("Animation Time");
      ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
      if (ImGui::BeginPopupModal("Animation Time", &m_camera_animation_duration_open))
      {
        ImGui::Text("Camera Name");
        ImGui::InputText("##textanimationtime", &m_string_animation_duration, ImGuiInputTextFlags_CharsDecimal);
        if (ImGui::Button("Ok"))
        {
          m_controller_app.get_simulation().set_simulation_duration(std::stof(m_string_animation_duration));
          m_string_animation_duration.clear();
          m_camera_animation_duration_open = false;
          ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Cancel"))
        {
          m_camera_animation_duration_open = false;
          m_string_animation_duration.clear();
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }

    // file loading
    if (m_file_dialog_open)
    {
      ImGui::OpenPopup("Open File");
      m_file_dialog_open = false;
    }

    if (fileDialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), m_file_extension_filter))
    {

      if (helper::ends_with_string(fileDialog.selected_fn, ".txt"))
      {
        VRDataLoader::load_txt_file(m_controller_app, fileDialog.selected_path);
      }
#ifdef WITH_TEEM
      else if (helper::ends_with_string(fileDialog.selected_fn, ".nrrd"))
      {
        std::vector<std::string> vals;
        vals.push_back(fileDialog.GetSelected().string());
          promises.push_back(new std::promise<Volume*>);
          futures.push_back(promises.back()->get_future());
          threads.push_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, promises.back()));
      }
#endif
    }

    if (m_file_dialog_save_dir)
    {
      ImGui::OpenPopup("Save File");
      m_file_dialog_save_dir = false;
      switch (m_current_save_modal)
      {
      case SAVE_SESSION:
        m_save_session_dialog_open = false;
        break;
      case SAVE_TRANSFER_FUNCTION:
        m_save_transfer_function_open = false;
        break;
      default:
        break;
      }
    }

    if (m_file_load_trnsf)
    {
      ImGui::OpenPopup("Load File");
      m_file_load_trnsf = false;
    }
    if (saveDialogLoadTrnsFnc.showFileDialog("Save File", imgui_addons::ImGuiFileBrowser::DialogMode::SELECT,
                                             ImVec2(700, 310)))
    {
      m_dir_to_save = saveDialogLoadTrnsFnc.selected_path;
      switch (m_current_save_modal)
      {
      case SAVE_SESSION:
        m_save_session_dialog_open = true;
        break;
      case SAVE_TRANSFER_FUNCTION:
        m_save_transfer_function_open = true;
        break;
      default:
        break;
      }
    }

    if (fileDialogLoadTrnsFnc.showFileDialog("Load File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                                             ImVec2(700, 310), m_file_extension_filter))
    {

      if (m_current_load_modal == LOAD_MODAL::LOAD_TRFR_FNC)
      {
        if (helper::ends_with_string(fileDialogLoadTrnsFnc.selected_fn, ".fnc"))
        {

          tfn_widget.clear();
          tfn_widget_multi.clear();
          m_tfns.clear();
          std::string filePath = fileDialogLoadTrnsFnc.selected_path;
          std::ifstream fileToLoad(filePath);
          load_transfer_functions(fileToLoad);
          m_trnfnc_table_selection = 0;
          m_current_load_modal = LOAD_NONE;
        }
      }
      if (m_current_load_modal == LOAD_MODAL::LOAD_SESSION)
      {
        if (helper::ends_with_string(fileDialogLoadTrnsFnc.selected_fn, ".usr"))
        {

          tfn_widget.clear();
          tfn_widget_multi.clear();
          m_tfns.clear();
          load_user_session(fileDialogLoadTrnsFnc.selected_path);
          m_trnfnc_table_selection = 0;
          m_current_load_modal = LOAD_NONE;
        }
      }
    }

    ImGui::End();
  }

  if (m_show_movie_saved_pop_up)
  {
    ImGui::OpenPopup("Movie saved##POPUP");
    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
    if (ImGui::BeginPopupModal("Movie saved##POPUP", &m_show_movie_saved_pop_up))
    {
      ImGui::Text("Movie saved");
      if (ImGui::Button("Ok"))
      {
        m_show_movie_saved_pop_up = false;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  if (m_show_clock)
  {
    ImGui::SetNextWindowPos(ImVec2(m_clock_pos_x, m_clock_pos_y));
    ImGui::SetNextWindowSize(ImVec2(m_clock_width, m_clock_height));
    ImGui::Begin("##clock", &m_show_clock, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
    ImGui::SetWindowFontScale(1.3f);
    ImGui::Text(m_time_info.c_str());
    ImGui::Text(m_day_info.c_str());
    ImGui::End();
  }

  if (m_use_transferfunction)
  {
    tfn_widget[0].draw_legend(0, m_legend_pos_y + 80, m_clock_width + 50, m_clock_height - 140);
  }
}

void UIView::init_ui(bool is2D, bool lookingGlass)
{

  if (!m_initialized)
  {
#ifdef WITH_NRRD
    fileDialog.SetTypeFilters({".txt", ".nrrd"});
#elseif
    fileDialog.SetTypeFilters({".txt"});
#endif

    float fontsize = 2.0;
    if (is2D)
    {
      fontsize = 1.0;
    }
    if (lookingGlass)
    {
      fontsize = 3.0;
    }
    std::cout << "is2d: " << (is2D ? "true" : "false") << std::endl;
    m_menu_handler = new VRMenuHandler(is2D);
    if (!m_menu_handler)
    {
      std::cout << "m_menu_handler: "
                << "NULL" << std::endl;
    }
    VRMenu *menu = m_menu_handler->addNewMenu(std::bind(&UIView::draw_ui_callback, this), 1024, 1024, 1, 1, fontsize);
    menu->setMenuPose(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 2, -1, 1));
    m_dir_to_save = m_controller_app.get_directory_path().c_str();

    for (int i = 0; i < MAX_COLUMS; ++i)
    {
      m_column_selected[i] = false;
    }

    m_initialized = true;
  }
}

void UIView::update_ui(int numVolumes)
{
  tfn_widget_multi.resize(1);
  tfn_widget.resize(1);
  m_use_transferfunction = true;
  m_selected_volume_TrFn.resize(1);
  m_selected_volume_TrFn[0].resize(numVolumes);
  for (int i = 0; i < numVolumes; i++)
  {
    m_selected_volume_TrFn[0][i] = false;
  }

  MyTransFerFunctions trfntc;
  char label[32];
  sprintf(label, "TF%d", m_trnfnct_counter++);
  trfntc.ID = tfn_widget.size();
  trfntc.Name = label;
  for (int i = 0; i < numVolumes; i++)
  {
    trfntc.volumes.push_back(false);
  }
  m_tfns.push_back(trfntc);
  m_trnfnc_table_selection = 0;

  load_ocean_color_maps();
}

void UIView::render_2D(Window_Properties &window_properties)
{

  m_clock_pos_x = window_properties.window_w - m_clock_width;
  m_clock_pos_y = window_properties.window_h - m_clock_height;
  m_legend_pos_y = window_properties.window_h - m_clock_height;

  m_menu_handler->drawMenu(window_properties.window_w, window_properties.window_h,
                           window_properties.framebuffer_w, window_properties.framebuffer_h);
}

void UIView::render_3D(glm::mat4 &space_matrix, Window_Properties &window_properties)
{
  // render menu

  if (m_show_menu)
  {

    m_menu_handler->drawMenu(window_properties.window_w, window_properties.window_h,
                             window_properties.framebuffer_w, window_properties.framebuffer_h);
  }
}

void UIView::update_3D_ui_frame()
{
  m_menu_handler->renderToTexture();
}

void UIView::set_cursor_pos(glm::vec2 &mPos)
{
  if (m_menu_handler)
  {
    m_menu_handler->setCursorPos(mPos.x, mPos.y);
  }
}

void UIView::set_analog_value(float value)
{
  if (m_menu_handler != NULL && m_menu_handler->windowIsActive())
  {
    m_menu_handler->setAnalogValue(value);
  }
}

int UIView::get_num_transfer_functions()
{
  return tfn_widget.size();
}

bool UIView::is_transfer_function_enabled(int tfn, int vol)
{
  return m_tfns[tfn].volumes[vol];
}

int UIView::get_render_method()
{
  return m_rendermethod;
}

bool UIView::is_render_volume_enabled()
{
  return m_renderVolume;
}

bool UIView::is_use_transfer_function_enabled()
{
  return m_use_transferfunction;
}

GLint UIView::get_transfer_function_colormap(int tfn)
{
  return tfn_widget[tfn].get_colormap_gpu();
}

GLint UIView::get_multitransfer_function_colormap(int tfn)
{
  return tfn_widget_multi[tfn].get_colormap_gpu();
}

int UIView::get_render_channel()
{
  return m_renderchannel;
}

void UIView::set_button_click(int button, int state)
{
  if (m_menu_handler && m_menu_handler->windowIsActive())
  {
    m_menu_handler->setButtonClick(button, state);
  }
}

void UIView::set_enable_render_volume()
{
  m_renderVolume = !m_renderVolume;
}

void UIView::set_controller_pose(glm::mat4 &pose)
{
  if (m_menu_handler)
  {
    m_menu_handler->setControllerPose(pose);
  }
}

void UIView::set_dynamic_slices(bool dynamicSlices)
{
  m_dynamic_slices = dynamicSlices;
}

bool UIView::is_dynamic_slices()
{
  return m_dynamic_slices;
}

void UIView::update_slices(float fps)
{
  if (fps < 25)
  {
    m_slices -= 5;
    m_slices = (m_slices < 10) ? 10 : m_slices;
  }
  else if (fps > 60)
  {
    m_slices += 5;
    m_slices = (m_slices > 1024) ? 1024 : m_slices;
  }
}

bool UIView::is_animated()
{
  return m_animated;
}

void UIView::set_is_animated(bool animated)
{
  m_animated = animated;
}

bool UIView::is_stopped()
{
  return m_stopped;
}

void UIView::update_animation(float speed, int numFrames)
{
  if (m_animated && !m_stopped)
  {
    m_ui_frame_controller += speed;
    if (m_ui_frame_controller > numFrames)
    {

      m_ui_frame_controller = 0.0f;
    }
    m_controller_app.set_frame(m_ui_frame_controller);
  }
}

void UIView::add_data_label(std::string &dataLabel)
{
  m_dataLabels.push_back(dataLabel);
}

void UIView::clear_data_labels()
{
  m_dataLabels.clear();
}

float UIView::get_z_scale()
{
  return m_z_scale;
}

float UIView::get_scale()
{
  return m_scale;
}

float UIView::get_slices()
{
  return m_slices;
}

float UIView::get_threshold()
{
  return m_threshold;
}

float UIView::get_multiplier()
{
  return m_multiplier;
}

bool UIView::is_ui_window_active()
{
  return m_menu_handler != nullptr && m_menu_handler->windowIsActive();
}

bool UIView::is_use_custom_clip_plane()
{
  return m_useCustomClipPlane;
}

glm::vec3 UIView::get_clip_min()
{
  return m_clip_min;
}

glm::vec3 UIView::get_clip_max()
{
  return m_clip_max;
}

void UIView::set_clip_min(glm::vec3 clip_min)
{
  m_clip_min = clip_min;
}

void UIView::set_clip_max(glm::vec3 clip_max)
{
  m_clip_max = clip_max;
}

void UIView::add_character(char c)
{
  if (m_transfer_function_options_window)
  {
    ImGui::ClearActiveID();
    m_copy_trnfnct_name += c;
  }
  if (m_save_transfer_function_open || m_save_session_dialog_open)
  {
    ImGui::ClearActiveID();
    m_save_file_name += c;
  }

  if (m_camera_name_window_open)
  {
    ImGui::ClearActiveID();
    m_copy_camera_name += c;
  }

  if (m_camera_animation_duration_open)
  {
    ImGui::ClearActiveID();
    if ((c >= '0' && c <= '9') || (c == '.'))
    {
      m_string_animation_duration += c;
    }
  }
}

void UIView::remove_character()
{
  if (m_transfer_function_options_window)
  {
    if (!m_copy_trnfnct_name.empty())
    {
      m_copy_trnfnct_name.pop_back();
    }
  }

  if (m_save_transfer_function_open || m_save_session_dialog_open)
  {
    if (!m_save_file_name.empty())
    {
      m_save_file_name.pop_back();
    }
  }

  if (m_camera_name_window_open)
  {
    if (!m_copy_camera_name.empty())
    {
      m_copy_camera_name.pop_back();
    }
  }

  if (m_camera_animation_duration_open)
  {
    if (!m_string_animation_duration.empty())
    {
      ImGui::ClearActiveID();
      m_string_animation_duration.pop_back();
    }
  }
}

void UIView::open_save_modal_dialog(std::string &id, bool &window_state,
                                    std::function<void(std::ofstream &)> save_function, std::string &extension)
{
  ImGui::OpenPopup(id.c_str());
  ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
  if (ImGui::BeginPopupModal(id.c_str(), &window_state))
  {

    ImGui::Text(m_dir_to_save.c_str());
    ImGui::SameLine();
    if (ImGui::Button("..."))
    {
      m_file_dialog_save_dir = true;
    }

    ImGui::InputText("##filename", &m_save_file_name);
    ImGui::IsItemActive();

    if (ImGui::Button("Save"))
    {
      std::string fileName = m_dir_to_save + helper::get_OS_path_separator() + m_save_file_name + extension;
      std::ofstream savefile(fileName);
      save_function(savefile);
      window_state = false;
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::Button("Cancel"))
    {
      window_state = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void UIView::add_transfer_function()
{
}

void UIView::save_transfer_functions(std::ofstream &saveFile)
{
  std::string pointsLine;
  if (saveFile.is_open())
  {
    saveFile << "numFunction " << std::to_string(m_tfns.size()) << "\n";
    for (int i = 0; i < m_tfns.size(); i++)
    {
      
      saveFile << "FuncName " + std::to_string(i + 1) + " " + m_tfns[i].Name + " " + std::to_string(tfn_widget[i].get_colormap_gpu()) + "\n";
      
      saveFile << "FuncPoints " << std::to_string(i + 1) << " ";
      for (int j = 0; j < tfn_widget[i].alpha_control_pts.size(); j++)
      {
        if (j != tfn_widget[i].alpha_control_pts.size() - 1)
        {
          saveFile << std::to_string(tfn_widget[i].alpha_control_pts[j].x) + "," + std::to_string(tfn_widget[i].alpha_control_pts[j].y) + ";";
        }
        else
        {
          saveFile << std::to_string(tfn_widget[i].alpha_control_pts[j].x) + "," + std::to_string(tfn_widget[i].alpha_control_pts[j].y);
        }
      }
      saveFile << "\n";
    }
  }
}

void UIView::save_user_session(std::ofstream &savefile)
{

  if (savefile.is_open())
  {
    if (!m_controller_app.get_loaded_file().empty())
    {
      savefile << "volume_loaded " << m_controller_app.get_loaded_file() << "\n";
    }
    savefile << "alpha_multiplier " << std::to_string(m_multiplier) << "\n";
    savefile << "threshold " << std::to_string(m_threshold) << "\n";
    savefile << "scale " << std::to_string(m_scale) << "\n";
    savefile << "z-scale " << std::to_string(m_z_scale) << "\n";
    savefile << "Slices " << std::to_string(m_slices) << "\n";
    savefile << "automatic _slice adjustment " << std::to_string(m_dynamic_slices) << "\n";
    savefile << "RenderMethod " << std::to_string(m_rendermethod) << "\n";
    savefile << "Render_Channel " << std::to_string(m_renderchannel) << "\n";
    savefile << "Render_Volume_data " << std::to_string(m_renderVolume) << "\n";

    savefile << "ClipXmin " << std::to_string(m_clip_min.x) << "\n";
    savefile << "ClipXmax " << std::to_string(m_clip_max.x) << "\n";
    savefile << "ClipYmin " << std::to_string(m_clip_min.y) << "\n";
    savefile << "ClipYmax " << std::to_string(m_clip_max.y) << "\n";
    savefile << "ClipZmin " << std::to_string(m_clip_min.z) << "\n";
    savefile << "ClipZmax " << std::to_string(m_clip_max.z) << "\n";
    savefile << "Use_transferfunction " << std::to_string(m_use_transferfunction) << "\n";

    int num_camera_poi = m_controller_app.get_simulation().get_simulation_states().size();
    if (num_camera_poi > 0)
    {
      savefile << "SIM " << num_camera_poi << "\n";
      save_simulation_states(savefile, num_camera_poi);
    }

    if (m_use_transferfunction)
    {
      savefile << "Trnfncs"
               << "\n";
      save_transfer_functions(savefile);
    }

    savefile.close();
  }
}

void UIView::load_transfer_functions(std::ifstream &loadFile)
{
  std::string line;

  int numFunctions = 0;

  if (loadFile.is_open())
  {
    while (std::getline(loadFile, line))
    {
      std::vector<std::string> vals;
      std::stringstream ss(line);
      std::string buf;

      while (ss >> buf)
      {
        vals.push_back(buf);
      }

      if (vals.size() > 0)
      {
        std::string tag = vals[0];

        if (tag == "numFunction")
        {
          numFunctions = std::stoi(vals[1]);
          tfn_widget.resize(numFunctions);
          tfn_widget_multi.resize(numFunctions);
          m_tfns.resize(numFunctions);
          m_trnfnct_counter = numFunctions + 1;
        }
        if (tag == "FuncName")
        {
          int index = std::stoi(vals[1]) - 1;
          m_tfns[index].Name = vals[2];
          int numVolumes = m_controller_app.get_num_volumes();
          for (int i = 0; i < numVolumes; i++)
          {
            m_tfns[index].volumes.push_back(false);
          }
          tfn_widget[index].selected_colormap = std::stoi(vals[3]);
          tfn_widget[index].update_colormap();
        }
        if (tag == "FuncPoints")
        {
          int index = std::stoi(vals[1]) - 1;
          std::string points = vals[2];
          size_t pos = 0;
          std::string token;
          std::string delimiter = ";";
          size_t last = 0;
          size_t next = 0;
          std::string point;
          int poinCounter = 0;
          tfn_widget[index].alpha_control_pts.clear();
          while ((next = points.find(delimiter, last)) != std::string::npos)
          {

            point = points.substr(last, next - last);
            int comaPos = point.find(",");
            float pointX = std::stof(point.substr(0, comaPos));
            float pointY = std::stof(point.substr(comaPos + 1));
            tfn_widget[index].alpha_control_pts.push_back(vec2f(pointX, pointY));

            last = next + 1;
            poinCounter++;
          }
          point = points.substr(last);
          int comaPos = point.find(",");
          float pointX = std::stof(point.substr(0, comaPos));
          float pointY = std::stof(point.substr(comaPos + 1));
          tfn_widget[index].alpha_control_pts.push_back(vec2f(pointX, pointY));
          std::cout << points.substr(last) << std::endl;
        }
      }
    }
    loadFile.close();
  }
}

void UIView::load_user_session(std::string filePath)
{
  std::string line;
  std::ifstream loadFile(filePath);

  if (loadFile.is_open())
  {
    while (std::getline(loadFile, line))
    {
      std::vector<std::string> vals; // Create vector to hold our words
      std::stringstream ss(line);
      std::string buf;

      while (ss >> buf)
      {
        vals.push_back(buf);
      }

      std::string tag = vals[0];

      if (tag == "volume_loaded")
      {
        std::string fileToLoad = vals[1];
        VRDataLoader::load_txt_file(m_controller_app, fileToLoad);
      }

      else if (tag == "alpha_multiplier")
      {
        m_multiplier = std::stof(vals[1]);
      }
      else if (tag == "threshold")
      {
        m_threshold = std::stof(vals[1]);
      }
      else if (tag == "scale")
      {
        m_scale = std::stof(vals[1]);
      }
      else if (tag == "z-scale")
      {
        m_z_scale = std::stof(vals[1]);
      }
      else if (tag == "Slices")
      {
        m_slices = std::stof(vals[1]);
      }
      else if (tag == "automatic _slice adjustment")
      {
        m_dynamic_slices = std::stof(vals[1]);
      }
      else if (tag == "RenderMethod")
      {
        m_rendermethod = std::stof(vals[1]);
      }
      else if (tag == "Render_Channel")
      {
        m_renderchannel = std::stof(vals[1]);
      }
      else if (tag == "Render_Volume_data")
      {
        m_renderVolume = std::stoi(vals[1]);
      }
      else if (tag == "ClipXmin")
      {
        m_clip_min.x = std::stof(vals[1]);
      }
      else if (tag == "ClipXmax")
      {
        m_clip_max.x = std::stof(vals[1]);
      }
      else if (tag == "ClipYmin")
      {
        m_clip_min.y = std::stof(vals[1]);
      }
      else if (tag == "ClipYmax")
      {
        m_clip_max.y = std::stof(vals[1]);
      }
      else if (tag == "ClipZmin")
      {
        m_clip_min.z = std::stof(vals[1]);
      }
      else if (tag == "ClipZmax")
      {
        m_clip_max.z = std::stof(vals[1]);
      }
      else if (tag == "Trnfncs")
      {
        load_transfer_functions(loadFile);
      }
      else if (tag == "POI")
      {
        load_camera_poi(loadFile, std::stoi(vals[1]));
      }
    }
    loadFile.close();
    m_controller_app.get_trackball_camera().reset_camera();
  }
}

void UIView::save_simulation_states(std::ofstream &saveFile, int num_camera_poi)
{
  if (saveFile.is_open())
  {

    auto sim_iterator = m_controller_app.get_simulation().get_simulation_states().begin();
    for (sim_iterator; sim_iterator != m_controller_app.get_simulation().get_simulation_states().end(); sim_iterator++)
    {
      saveFile << sim_iterator->poi.label << " "
               << std::to_string(sim_iterator->poi.eye.x) + " " + std::to_string(sim_iterator->poi.eye.y) + " " + std::to_string(sim_iterator->poi.eye.z) + " "
               << std::to_string(sim_iterator->poi.target.x) + " " + std::to_string(sim_iterator->poi.target.y) + " " + std::to_string(sim_iterator->poi.target.z) + " "
               << std::to_string(sim_iterator->poi.up.x) + " " + std::to_string(sim_iterator->poi.up.y) + " " + std::to_string(sim_iterator->poi.up.z) + " "
               << std::to_string(sim_iterator->poi.radius) + " "
               << std::to_string(sim_iterator->min_clip.x) + " " + std::to_string(sim_iterator->min_clip.y) + " " + std::to_string(sim_iterator->min_clip.z) + " "
               << std::to_string(sim_iterator->max_clip.x) + " " + std::to_string(sim_iterator->max_clip.y) + " " + std::to_string(sim_iterator->max_clip.z) + "\n";
    }
  }
}

void UIView::load_camera_poi(std::ifstream &loadFile, int num_poi)
{
  std::string line;
  if (loadFile.is_open())
  {
    for (int i = 0; i < num_poi; ++i)
    {
      std::getline(loadFile, line);
      std::vector<std::string> poiVals;
      read_file_line(line, poiVals);

      if (poiVals.size() > 0)
      {
        std::string label = poiVals[0];
        float eye_x = std::stof(poiVals[1]);
        float eye_y = std::stof(poiVals[2]);
        float eye_z = std::stof(poiVals[3]);
        float target_x = std::stof(poiVals[4]);
        float target_y = std::stof(poiVals[5]);
        float target_z = std::stof(poiVals[6]);
        float up_x = std::stof(poiVals[7]);
        float up_y = std::stof(poiVals[8]);
        float up_z = std::stof(poiVals[9]);
        float radius = std::stof(poiVals[10]);
        float min_clip_x = std::stof(poiVals[11]);
        float min_clip_y = std::stof(poiVals[12]);
        float min_clip_z = std::stof(poiVals[13]);
        float max_clip_x = std::stof(poiVals[14]);
        float max_clip_y = std::stof(poiVals[15]);
        float max_clip_z = std::stof(poiVals[16]);

        SimulationState sim;
        sim.poi.label = label;
        sim.poi.eye = glm::vec3(eye_x, eye_y, eye_z);
        sim.poi.target = glm::vec3(target_x, target_y, target_z);
        sim.poi.up = glm::vec3(up_x, up_y, up_z);
        sim.poi.radius = radius;
        sim.min_clip = glm::vec3(min_clip_x, min_clip_y, min_clip_z);
        sim.max_clip = glm::vec3(max_clip_x, max_clip_y, max_clip_z);
        m_controller_app.get_simulation().add_simulation_state(sim);
      }
    }
  }
}

void UIView::read_file_line(std::string &line, std::vector<std::string> &values)
{
  std::stringstream ss(line);
  std::string buf;

  while (ss >> buf)
  {
    values.push_back(buf);
  }
}

void UIView::set_animation_length(int num_frames)
{
  m_num_animation_frames = num_frames;
}

void UIView::adjust_transfer_function_to_histogram()
{

  std::vector<float> histogram_copy;

  for (int j = 0; j < m_histogram.getHistogram().size(); j++)
  {
    if (m_histogram.getHistogram()[j] > 0.0005)
    {
      histogram_copy.push_back(m_histogram.getHistogram()[j]);
    }
  }

  const std::size_t pos1 = std::floor(m_histogram_quantiles[0] * std::distance(histogram_copy.begin(), histogram_copy.end()));
  const std::size_t pos2 = std::floor(m_histogram_quantiles[1] * std::distance(histogram_copy.begin(), histogram_copy.end()));
  if (histogram_copy.size() > pos1 && histogram_copy.size() > pos2)
  {
    std::nth_element(histogram_copy.begin(), histogram_copy.begin() + pos1, histogram_copy.end());

    std::vector<float>::iterator it1 = std::find(m_histogram.getHistogram().begin(), m_histogram.getHistogram().end(), histogram_copy[pos1]);
    float index1 = std::distance(m_histogram.getHistogram().begin(), it1) / 256.f;

    m_histogram_point_1.x = index1;
    m_histogram_point_1.y = 0.0f;

    std::nth_element(histogram_copy.begin(), histogram_copy.begin() + pos2, histogram_copy.end());

    std::vector<float>::iterator it2 = std::find(m_histogram.getHistogram().begin(), m_histogram.getHistogram().end(), histogram_copy[pos2]);
    float index2 = std::distance(m_histogram.getHistogram().begin(), it2) / 256.f;

    m_histogram_point_2.x = index2;
    m_histogram_point_2.y = 1.0f;
  }
}

void UIView::compute_new_histogram_view()
{
  if (!m_compute_new_histogram)
  {
    return;
  }

  std::vector<float> histogram(256, 0.f);
  float global_min = std::numeric_limits<float>::max();
  float global_max = 0.f;
  for (int i = 0; i < m_controller_app.get_num_volumes(); i++)
  {

    if (m_column_selected[i])
    {
      if (m_controller_app.get_volume(i)[0]->getMin() < global_min)
      {
        global_min = m_controller_app.get_volume(i)[0]->getMin();
      }
      if (m_controller_app.get_volume(i)[0]->getMax() > global_max)
      {
        global_max = m_controller_app.get_volume(i)[0]->getMax();
      }

      m_histogram.setMinMax(global_min, global_max);
      std::vector<float> current_histogram = m_controller_app.get_volume(i)[0]->getHistogram(0);
      for (int j = 0; j < current_histogram.size(); j++)
      {
        histogram[j] += current_histogram[j];
      }
    }
  }

  m_histogram.setHistogram(histogram);

  m_compute_new_histogram = false;
}

void UIView::add_transfer_function()
{
  int numVolumes = m_controller_app.get_num_volumes();
  MyTransFerFunctions trfntc;
  char label[32];
  sprintf(label, "TF%d", (int)tfn_widget.size());
  trfntc.ID = tfn_widget.size();
  trfntc.Name = label;
  for (int i = 0; i < numVolumes; i++)
  {
    trfntc.volumes.push_back(false);
  }
  m_tfns.push_back(trfntc);
}

void UIView::get_quantiles(int row)
{
  m_copy_trnfnct_name = m_tfns[row].Name;
  float q_min = 0;
  float q_max = 0;
  tfn_widget[m_trnfnc_table_selection].get_quantiles(q_min, q_max);
  m_histogram_quantiles[0] = q_min;
  m_histogram_quantiles[1] = q_max;
  m_transfer_function_options_window = true;
}

void UIView::set_volume_time_info(time_t time)
{

  tm *time_info = localtime(&time);
  if (time_info)
  {
    bool pm = time_info->tm_hour >= 12;
    int hour_12 = (time_info->tm_hour >= 13) ? time_info->tm_hour - 12 : time_info->tm_hour;

    std::stringstream ss_time;
    ss_time << std::setw(2) << std::setfill('0') << hour_12 << ":";
    ss_time << std::setw(2) << std::setfill('0') << time_info->tm_min << " ";
    ss_time << (pm ? "PM" : "AM");

    std::stringstream ss_day;
    ss_day << m_months[time_info->tm_mon] << " ";
    ss_day << std::setw(2) << std::setfill('0') << time_info->tm_mday;
    ss_day << ", " << time_info->tm_year + 1900;

    m_time_info = ss_time.str();
    m_day_info = ss_day.str();
  }
}

void UIView::draw_transfer_function_legend()
{
  if (m_use_transferfunction)
  {
    tfn_widget[0].draw_legend();
  }
}

void UIView::set_transfer_function_min_max(float min, float max)
{
  if (m_use_transferfunction)
  {
    tfn_widget[0].setMinMax(min, max);
  }
}

void UIView::load_ocean_color_maps()
{
  int w, h, n;
  int comp;

  for (std::string color_map_name : m_ocean_color_maps_names)
  {
    std::string file_name_path = m_controller_app.get_directory_path() + OS_SLASH + m_color_map_directory + OS_SLASH + color_map_name;
    uint8_t *img_data = stbi_load(file_name_path.c_str(), &w, &h, &comp, 4);
    auto img = std::vector<uint8_t>(img_data, img_data + w * 1 * 4);
    stbi_image_free(img_data);

    // set name

    std::string name = color_map_name.substr(0, color_map_name.find_first_of("."));

    Colormap color_map(name, img);
    tfn_widget[0].add_colormap(color_map);
  }
}
