#include "UI/UIView.h"
#include "UI/UIController.h"
#include "interaction/CreateMovieAction.h"
#include "interaction/HelperFunctions.h"

#include "vrapp/VRVolumeApp.h"

#include <glm/gtc/type_ptr.hpp>

UIView::UIView(VRVolumeApp& controllerApp):m_controller_app(controllerApp), m_multiplier( 1.0f ), m_threshold{ 0.0f },
m_z_scale{ 0.16f }, m_scale{ 1.0f }, m_slices(256), m_dynamic_slices{ false }, m_renderVolume(true), m_selectedTrnFnc(0),
m_animated(false), m_ui_frame_controller{ 0.0f }, m_menu_handler(nullptr), m_initialized(false), m_use_transferfunction( false ),
m_clip_max( 1.0 ), m_clip_min( 0.0 ), m_clip_ypr( 0.0 ), m_clip_pos( 0.0 ), m_useCustomClipPlane( false ), m_rendermethod( 1 ), m_renderchannel( 0 )
{

}

void UIView::draw_ui_callback()
{
  if (!m_initialized)
  {
    return;
  }

  ImGui::Begin("Volumeviewer");
  ImGui::BeginTabBar("##tabs");
  if (ImGui::BeginTabItem("General"))
  {
     if (ImGui::Button("load file", ImVec2(ImGui::GetWindowSize().x * 0.5f - 1.5 * ImGui::GetStyle().ItemSpacing.x, 0.0f)))
         fileDialog.Open();
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

     ImGui::SameLine(ImGui::GetWindowSize().x * 0.5f, 0);
     ImGui::Text("FPS = %f", m_controller_app.get_fps());
     const char* items[] = { "sliced" , "raycast" };
     ImGui::Combo("RenderMethod", &m_rendermethod, items, IM_ARRAYSIZE(items));

     const char* items_channel[] = { "based on data" , "red", "green" , "blue", "alpha", "rgba", "rgba with alpha as max rgb" };
     ImGui::Combo("Render Channel", &m_renderchannel, items_channel, IM_ARRAYSIZE(items_channel));

     ImGui::Checkbox("Render Volume data", &m_renderVolume);

     int numVolumes = m_controller_app.get_num_volumes();

    

    if (numVolumes > 0)
    {
      if (ImGui::SmallButton("New")) {
        tfn_widget.push_back(TransferFunctionWidget());
        tfn_widget_multi.push_back(TransferFunctionMultiChannelWidget());
        int index = m_selectedTrFn.size();
        m_selectedTrFn.push_back(std::vector<bool>(numVolumes));
        for (int i = 0; i < numVolumes; i++)
        {
          m_selectedTrFn[index][i] = false;
        }
      };
      ImGui::SameLine();
      if (ImGui::SmallButton("Remove")) {


      };

      ImGui::BeginTable("##Transfer Function Editor", 3);
      ImGui::TableSetupColumn("Name");
      for (int column = 0; column < numVolumes; column++)
      {
        ImGui::TableSetupColumn(m_dataLabels[column].c_str());
      }
      ImGui::TableHeadersRow();

      for (int row = 0; row < tfn_widget.size(); row++)
      {
        ImGui::TableNextRow();
        for (int col = 0; col < numVolumes + 1; col++)
        {
          ImGui::TableSetColumnIndex(col);
          if (col == 0)
          {
            char buf[32];
            sprintf(buf, "TF%d", row);
            if (ImGui::SmallButton(buf)) {
              std::cout << buf << std::endl;
              m_selectedTrnFnc = row;
            };

          }
          else
          {
            char buf[32];
            sprintf(buf, "##On%d%d", col, row);
            bool b = m_selectedTrFn[row][col - 1];
            ImGui::Checkbox(buf, &b);
            m_selectedTrFn[row][col - 1] = b;
            if (b)
            {
              std::cout << "ddd" << std::endl;
            }
          }

        }
      }

      ImGui::EndTable();

      ImGui::Checkbox("use transferfunction", &m_use_transferfunction);
      if (m_use_transferfunction) {
        bool is_multi_channel = m_controller_app.data_is_multi_channel();
        if (is_multi_channel)
        {
          for (int i = 0; i < 3; i++) {
            if (m_animated)
            {
              /*	
                frame by frame animation

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
            else {
              /*		tfn_widget_multi[m_selectedVolume].setHistogram(m_volumes[m_selectedVolume][0]->getTransferfunction(i), i);*/
            }
          }
          m_controller_app.set_multi_transfer( true);
          tfn_widget_multi[m_selectedTrnFnc].draw_ui();
        }
        else
        {
          if (m_animated)
          {
            /*	
             frame by frame animation

            unsigned int active_volume = floor(m_frame);
              unsigned int active_volume2 = ceil(m_frame);
              double alpha = m_frame - active_volume;
              tfn_widget[m_selectedVolume].setMinMax(m_volumes[m_selectedVolume][active_volume]->getMin() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMin() * (1.0 - alpha),
                m_volumes[m_selectedVolume][active_volume]->getMax() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMax() * (1.0 - alpha));
              if (active_volume < m_volumes[m_selectedVolume].size() && active_volume2 < m_volumes[m_selectedVolume].size())
                tfn_widget[m_selectedVolume].setBlendedHistogram(m_volumes[m_selectedVolume][active_volume]->getTransferfunction(0), m_volumes[m_selectedVolume][active_volume2]->getTransferfunction(0), alpha);*/

          }
          else if (numVolumes > 0) {
            /*tfn_widget[m_selectedVolume].setHistogram(m_volumes[m_selectedVolume][0]->getTransferfunction(0));
            tfn_widget[m_selectedVolume].setMinMax(m_volumes[m_selectedVolume][0]->getMin(), m_volumes[m_selectedVolume][0]->getMax());*/
          }
          m_controller_app.set_multi_transfer(false);
          tfn_widget[m_selectedTrnFnc].draw_ui();
        }
      }

    }


    if (m_animated) {
      ImGui::Text("Timestep");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(-100 - ImGui::GetStyle().ItemSpacing.x);
      float frame_tmp = m_controller_app.get_current_frame() + 1;
      //controls animated multi datasets
      //ImGui::SliderFloat("##Timestep", &frame_tmp, 1, m_volumes[m_selectedVolume].size());
      m_ui_frame_controller = frame_tmp - 1;
      m_controller_app.set_frame(m_ui_frame_controller);
      ImGui::SameLine();

      std::string text = m_stopped ? "Play" : "Stop";
      if (ImGui::Button(text.c_str(), ImVec2(100, 0))) {
        m_stopped = !m_stopped;
      }


      if (ImGui::Button("Write Movie"))
      {
        m_controller_app.run_movie();
      }
    }
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem("Clipping"))
  {
    ImGui::Text("Axis aligned clip");
    glm::vec2 bound = { m_clip_min.x * 100 ,m_clip_max.x * 100 };
    ImGui::DragFloatRange2("X", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
    m_clip_min.x = bound.x / 100;
    m_clip_max.x = bound.y / 100;

    bound = { m_clip_min.y * 100 ,m_clip_max.y * 100 };
    ImGui::DragFloatRange2("Y", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
    m_clip_min.y = bound.x / 100;
    m_clip_max.y = bound.y / 100;

    bound = { m_clip_min.z * 100 ,m_clip_max.z * 100 };
    ImGui::DragFloatRange2("Z", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
    m_clip_min.z = bound.x / 100;
    m_clip_max.z = bound.y / 100;

    if (ImGui::Button("Reset")) {
      m_clip_min = glm::vec3(0.0f);
      m_clip_max = glm::vec3(1.0f);
    }
    
    ImGui::Checkbox("Custom Clipping plane", &m_useCustomClipPlane);
    if (m_useCustomClipPlane) {
      ImGui::SliderAngle("Pitch", &m_clip_ypr.y, -90, 90);
      ImGui::SliderAngle("Roll", &m_clip_ypr.z, -180, 180);

      ImGui::SliderFloat("Position X", &m_clip_pos.x, -0.5, 0.5);
      ImGui::SliderFloat("Position y", &m_clip_pos.y, -0.5, 0.5);
      ImGui::SliderFloat("Position z", &m_clip_pos.z, -0.5, 0.5);
      if (ImGui::Button("Reset##Reset2")) {
        m_clip_ypr = glm::vec3(0.0f);
        m_clip_pos = glm::vec3(0.0f);
      }
      
    }
    ImGui::EndTabItem();
  }
  ImGui::EndTabBar();
     
     //file loading
  fileDialog.Display();

  if (fileDialog.HasSelected())
  {
    if (helper::ends_with_string(fileDialog.GetSelected().string(), ".txt"))
    {
      m_controller_app.load_txt_file(fileDialog.GetSelected().string());
    }
#ifdef WITH_TEEM
    else if (helper::ends_with_string(fileDialog.GetSelected().string(), ".nrrd")) {
      std::vector<std::string> vals;
      /*	vals.push_back(fileDialog.GetSelected().string());
        promises.push_back(new std::promise<Volume*>);
        futures.push_back(promises.back()->get_future());
        threads.push_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, promises.back()));*/
    }
#endif
    fileDialog.ClearSelected();
  }
    
   
  
  ImGui::End();
}

void UIView::init_ui(bool is2D, bool lookingGlass)
{

  if (!m_initialized)
  {
    fileDialog.SetTitle("load data");
#ifdef WITH_NRRD
    fileDialog.SetTypeFilters({ ".txt", ".nrrd" });
    #elseif
      fileDialog.SetTypeFilters({ ".txt" });
#endif

    float fontsize = 2.0;
    if (is2D) {
      fontsize = 1.0;
    }
    if (lookingGlass) {
      fontsize = 3.0;
    }
    m_menu_handler = new VRMenuHandler(is2D);
    VRMenu* menu = m_menu_handler->addNewMenu(std::bind(&UIView::draw_ui_callback, this), 1024, 1024, 1, 1, fontsize);
    menu->setMenuPose(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 2, -1, 1));
    m_initialized = true;
  }
  
  
}

void UIView::update_ui(int numVolumes)
{
  tfn_widget_multi.resize(1);
  tfn_widget.resize(1);
  m_selectedTrFn.resize(1);
  m_selectedTrFn[0].resize(numVolumes);
  for (int i = 0; i < numVolumes; i++)
  {
    m_selectedTrFn[0][i] = false;
  }
}

void UIView::render_2D()
{
  if (m_show_menu ) 
  {
    m_menu_handler->drawMenu();
    if (m_use_transferfunction) {
      tfn_widget[m_selectedTrnFnc].drawLegend();
    }
  }
  
}

void UIView::render_3D(glm::mat4& space_matrix)
{
  //render menu	
  
  if (m_show_menu)
  {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(space_matrix));
    m_menu_handler->drawMenu();
  }
    
}


void UIView::update_3D_ui_frame()
{
  m_menu_handler->renderToTexture();
}

void UIView::set_cursor_pos(glm::vec2& mPos)
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
  return m_selectedTrFn[tfn][vol];
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

void UIView::set_controller_pose(glm::mat4& pose)
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

void UIView::add_data_label(std::string& dataLabel)
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
  return m_menu_handler != NULL && m_menu_handler->windowIsActive();
}

bool UIView::is_use_custom_clip_plane()
{
  return m_useCustomClipPlane;
}

