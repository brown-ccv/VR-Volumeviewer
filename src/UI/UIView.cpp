#include "UI/UIView.h"

#include "interaction/CreateMovieAction.h"
#include "interaction/HelperFunctions.h"

#include "vrapp/VRVolumeApp.h"

#include <glm/gtc/type_ptr.hpp>

#include "loader/VRDataLoader.h"

#include <fstream>

UIView::UIView(VRVolumeApp& controllerApp) :m_controller_app(controllerApp), m_multiplier(1.0f), m_threshold(0.0f),
m_z_scale(0.16f), m_scale{ 1.0f }, m_slices(256), m_dynamic_slices(false), m_renderVolume(true), m_selectedTrnFnc(0),
m_animated(false), m_ui_frame_controller(0.0f), m_menu_handler(nullptr), m_initialized(false), m_use_transferfunction(false),
m_clip_max(1.0), m_clip_min(0.0), m_clip_ypr(0.0), m_clip_pos(0.0), m_useCustomClipPlane(false), m_rendermethod(1), m_renderchannel(0),
m_table_selection(-1), m_modal_trnfct_open(false), m_save_trnfct_open(false), m_trnfnct_counter(1), m_file_dialog_open(false),
m_file_load_trnsf(false),m_show_menu(true)

{
  
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

  ImGui::Begin("Volumeviewer");
  ImGui::BeginTabBar("##tabs");
  if (ImGui::BeginTabItem("General"))
  {
    if (ImGui::Button("load file", ImVec2(ImGui::GetWindowSize().x * 0.5f - 1.5 * ImGui::GetStyle().ItemSpacing.x, 0.0f)))
    {
      m_file_dialog_open = true;
      //fileDialog.Open();
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
        int index = m_selected_volume_TrFn.size();
        m_selected_volume_TrFn.push_back(std::vector<bool>(numVolumes));
        for (int i = 0; i < numVolumes; i++)
        {
          m_selected_volume_TrFn[index][i] = false;
        }

        MyTransFerFunctions trfntc;
        char label[32];

        sprintf(label, "TF%d", m_trnfnct_counter++);
        trfntc.ID =  tfn_widget.size();
        trfntc.Name = label;
        for (int i = 0; i < numVolumes; i++)
        {
          trfntc.volumes.push_back(false);
        }
        m_tfns.push_back(trfntc);
      };
      ImGui::SameLine();
      if (ImGui::SmallButton("Remove")) {
        
        if (m_tfns.size() == 1)
        {
          //there should be one by default
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
          m_table_selection = 0;
        }
        else if (m_tfns.size() > 1 && m_table_selection >= 0)
        {
          tfn_widget.erase(tfn_widget.begin() + m_table_selection); ;
          tfn_widget_multi.erase(tfn_widget_multi.begin() + m_table_selection);
          m_tfns.erase(m_tfns.begin() + m_table_selection);
          if (m_table_selection != 0)
          {
            m_table_selection = m_table_selection - 1;
          }
          
        }
      };
      ImGui::SameLine();
      if (ImGui::SmallButton("Clear")) {
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
        m_table_selection = 0;
      };

      ImGui::SameLine();
      if (ImGui::SmallButton("Save")) {
        m_save_trnfct_open = true;
      };

      ImGui::SameLine();
      if (ImGui::SmallButton("Load")) {
        
        m_file_load_trnsf = true;
        
      };

      int selection = 0;
      
      ImGui::BeginTable("##Transfer Function Editor", numVolumes+1, ImGuiTableFlags_Borders);
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
      
      for (int column = 0; column < numVolumes; column++)
      {
        ImGui::TableSetupColumn(m_dataLabels[column].c_str());
      }
      ImGui::TableHeadersRow();
      
      for (int row = 0; row < m_tfns.size(); row++)
      {
        ImGui::TableNextRow();
        for (int col = 0; col < numVolumes + 1; col++)
        {
          ImGui::TableSetColumnIndex(col);
          
          if (col == 0)
          {
           
            ImGuiSelectableFlags selectable_flags =  ImGuiSelectableFlags_AllowDoubleClick;
            bool item_is_selected = (row == m_table_selection) ? true : false;
            if (ImGui::Selectable(m_tfns[row].Name.c_str(), item_is_selected, selectable_flags))
            {
              m_table_selection = row;
              
              if (ImGui::IsMouseDoubleClicked(0))
              {
                m_copy_trnfnct_name = m_tfns[row].Name;
                m_modal_trnfct_open = true;
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
            if (b)
            {
              std::cout << "ddd" << std::endl;
            }
          }

        }
      }

      ImGui::EndTable();


      if (m_save_trnfct_open)
      {
        ImGui::OpenPopup("Save Transfer Functions");
        ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal("Save Transfer Functions", &m_save_trnfct_open))
        {
          char* writable = new char[m_save_file_name.size() + 1];
          std::copy(m_save_file_name.begin(), m_save_file_name.end(), writable);
          writable[m_save_file_name.size()] = '\0'; // don't forget the terminating 0
          //ImGui::PushItemWidth(-1);
          ImGui::InputText("##text2", writable, IM_ARRAYSIZE(writable));
          ImGui::IsItemActive();
          
          /*
          ImGui::SameLine();
          if (ImGui::Button("dir"))
          {
            fileDialogSave->Open();
          }*/
          //ImGui::PopItemWidth();

          if (ImGui::Button("Save"))
          {
            save_trans_functions();
            m_save_trnfct_open = false;
            ImGui::CloseCurrentPopup();
          }
          if (ImGui::Button("Cancel"))
          {
            m_save_trnfct_open = false;
            ImGui::CloseCurrentPopup();
          }

          ImGui::EndPopup();
        }
      }
    





      if (m_modal_trnfct_open)
      {
        ImGui::OpenPopup("Modal window");
        ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal("Modal window", &m_modal_trnfct_open))
        {
          char* writable = new char[m_copy_trnfnct_name.size() + 1];
          std::copy(m_copy_trnfnct_name.begin(), m_copy_trnfnct_name.end(), writable);
          writable[m_copy_trnfnct_name.size()] = '\0'; // don't forget the terminating 0
          //ImGui::PushItemWidth(-1);
          ImGui::InputText("##text1", writable, IM_ARRAYSIZE(writable));
          ImGui::IsItemActive();
          //ImGui::PopItemWidth();

          if (ImGui::Button("Save"))
          {
            m_tfns[m_table_selection].Name = m_copy_trnfnct_name;
            m_modal_trnfct_open = false;
            ImGui::CloseCurrentPopup();
          }
          if (ImGui::Button("Cancel"))
          {
            m_modal_trnfct_open = false;
            ImGui::CloseCurrentPopup();
          }

          ImGui::EndPopup();
        }
      }
     


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
          m_controller_app.set_multi_transfer(true);
          //tfn_widget_multi[m_selectedTrnFnc].draw_histogram();
          tfn_widget_multi[m_table_selection].draw_ui();
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
         // tfn_widget[m_selectedTrnFnc].draw_histogram();
          tfn_widget[m_table_selection].draw_ui();
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

      #if (!defined(__APPLE__))
      if (ImGui::Button("Write Movie"))
      {
        m_controller_app.run_movie();
      }
      #endif
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
  if (m_file_dialog_open)
  {
    ImGui::OpenPopup("Open File");
    m_file_dialog_open = false;
  }

  
   

  if (fileDialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310)))
  {
   
    if (helper::ends_with_string(fileDialog.selected_fn, ".txt"))
    {
      VRDataLoader::get_instance()->load_txt_file(m_controller_app, fileDialog.selected_path);
      // m_controller_app.load_txt_file(fileDialog.GetSelected().string());
    }
    #ifdef WITH_TEEM
    else if (helper::ends_with_string(fileDialog.selected_fn, ".nrrd")) {
      std::vector<std::string> vals;
      /*	vals.push_back(fileDialog.GetSelected().string());
        promises.push_back(new std::promise<Volume*>);
        futures.push_back(promises.back()->get_future());
        threads.push_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, promises.back()));*/
    }
    #endif
  }

  //fileDialog.Display();

//  if (fileDialog.HasSelected())
//  {
//    if (helper::ends_with_string(fileDialog.GetSelected().string(), ".txt"))
//    {
//      VRDataLoader::get_instance()->load_txt_file(m_controller_app, fileDialog.GetSelected().string());
//      // m_controller_app.load_txt_file(fileDialog.GetSelected().string());
//    }
//#ifdef WITH_TEEM
//    else if (helper::ends_with_string(fileDialog.GetSelected().string(), ".nrrd")) {
//      std::vector<std::string> vals;
//      /*	vals.push_back(fileDialog.GetSelected().string());
//        promises.push_back(new std::promise<Volume*>);
//        futures.push_back(promises.back()->get_future());
//        threads.push_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, promises.back()));*/
//    }
//#endif
//    fileDialog.ClearSelected();
//  }


  if (m_file_load_trnsf)
  {
    ImGui::OpenPopup("Load File");
    m_file_load_trnsf = false;

  }

  if (fileDialogLoadTrnsFnc.showFileDialog("Load File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310)))
  {
    
    if (helper::ends_with_string(fileDialogLoadTrnsFnc.selected_fn, ".fnc"))
    {

      tfn_widget.clear();
      tfn_widget_multi.clear();
      m_tfns.clear();
      load_trans_functions(fileDialogLoadTrnsFnc.selected_path);
      m_table_selection = 0;

    }
  }

  //fileDialogLoadTrnsFnc->Display();


  //if (fileDialogLoadTrnsFnc->HasSelected())
  //{
  //  //std::cout << "Selected filename" << fileDialogLoadTrnsFnc->GetSelected().string() << std::endl;
  //  if (helper::ends_with_string(fileDialogLoadTrnsFnc->GetSelected().string(), ".fnc"))
  //  {

  //    tfn_widget.clear();
  //    tfn_widget_multi.clear();
  //    m_tfns.clear();
  //    load_trans_functions(fileDialogLoadTrnsFnc->GetSelected().string());
  //    m_table_selection = 0;
  //    
  //  }
  //  
  //  
  //  fileDialogLoadTrnsFnc->ClearSelected();
  //}


  ImGui::End();
}

void UIView::init_ui(bool is2D, bool lookingGlass)
{

  if (!m_initialized)
  {
    //fileDialog.SetTitle("load data");
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
    std::cout << "is2d: " << (is2D? "true":"false") << std::endl;
    m_menu_handler = new VRMenuHandler(is2D);
    if(!m_menu_handler)
    { std::cout << "m_menu_handler: " << "NULL" << std::endl;
   
    }
    VRMenu* menu = m_menu_handler->addNewMenu(std::bind(&UIView::draw_ui_callback, this), 1024, 1024, 1, 1, fontsize);
    menu->setMenuPose(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 2, -1, 1));
    m_initialized = true;
    std::cout << "end init ui" << std::endl;
  }


}

void UIView::update_ui(int numVolumes)
{
  tfn_widget_multi.resize(1);
  tfn_widget.resize(1);
 /* m_selected_volume_TrFn.resize(1);
  m_selected_volume_TrFn[0].resize(numVolumes);
  for (int i = 0; i < numVolumes; i++)
  {
    m_selected_volume_TrFn[0][i] = false;
  }*/

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
  m_table_selection = 0;
}

void UIView::render_2D( int window_width,int window_height
                  ,int framebuffer_width,int framebuffer_height)
{
     
  if (m_show_menu)
  {
    m_menu_handler->drawMenu(window_width,window_height,framebuffer_width,framebuffer_height);
    if (m_use_transferfunction) {
      tfn_widget[m_selectedTrnFnc].drawLegend();
    }
  }

}

void UIView::render_3D(glm::mat4& space_matrix, int window_width,int window_height
                  ,int framebuffer_width,int framebuffer_height)
{
  //render menu	

  if (m_show_menu)
  {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(space_matrix));
    m_menu_handler->drawMenu(window_width,window_height,framebuffer_width,framebuffer_height);
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
  //return m_selected_volume_TrFn[tfn][vol];
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

void UIView::set_chracter(char c)
{
  if (m_modal_trnfct_open)
  {
    m_copy_trnfnct_name += c;
  }
  if (m_save_trnfct_open)
  {
    m_save_file_name += c;
  }
}

void UIView::remove_character()
{
  if (m_modal_trnfct_open)
  {
    m_copy_trnfnct_name.pop_back();
  }
  
  if (m_save_trnfct_open)
  {
    m_save_file_name.pop_back();
  }
}

void UIView::add_trans_function()
{

}

void UIView::save_trans_functions()
{
  std::string fileName = m_save_file_name + ".fnc";
  std::ofstream savefile(fileName);
  std::string pointsLine;
  if (savefile.is_open())
  {
    savefile << "numFunction " << std::to_string(m_tfns.size()) << "\n";
    for(int i = 0; i < m_tfns.size();i++)
    {
      //savefile << m_tfns[i].ID + ",";
      savefile << "FuncName " + std::to_string(i+1) + " " + m_tfns[i].Name + " " + std::to_string(tfn_widget[i].get_colormap_gpu()) +"\n";
   /*   for (int j = 0; j < tfn_widget_multi[i].alpha_control_pts.size(); j++)
      {
        if (j != tfn_widget_multi[i].alpha_control_pts.size()-1)
        {
          savefile << std::to_string(tfn_widget_multi[i].alpha_control_pts[j][0].x) +","+ std::to_string(tfn_widget_multi[i].alpha_control_pts[j][0].y) +
            std::to_string(tfn_widget_multi[i].alpha_control_pts[j][1].x) + "," + std::to_string(tfn_widget_multi[i].alpha_control_pts[j][1].y);
        }
        else
        {
          savefile << std::to_string(tfn_widget_multi[i].alpha_control_pts[j][0].x) + "," + std::to_string(tfn_widget_multi[i].alpha_control_pts[j][0].y )+
            std::to_string(tfn_widget_multi[i].alpha_control_pts[j][1].x) + "," + std::to_string(tfn_widget_multi[i].alpha_control_pts[j][1].y) +";";
        }
        
      }*/
      savefile << "FuncPoints " << std::to_string(i+1) << " ";
      for (int j = 0; j < tfn_widget[i].alpha_control_pts.size(); j++)
      {
        if (j != tfn_widget[i].alpha_control_pts.size() - 1)
        {
          savefile << std::to_string(tfn_widget[i].alpha_control_pts[j].x) + "," + std::to_string(tfn_widget[i].alpha_control_pts[j].y) + ";";
        }
        else
        {
          savefile << std::to_string(tfn_widget[i].alpha_control_pts[j].x) + "," + std::to_string(tfn_widget[i].alpha_control_pts[j].y) ;
        }

      }
      savefile << "\n";
    }
    savefile.close();
  }
}

void UIView::load_trans_functions(std::string filePath)
{
  std::string line;
  std::ifstream loadFile(filePath);
  int numFunctions = 0;

  if (loadFile.is_open())
  {
    while (std::getline(loadFile, line))
    {
      std::vector<std::string> vals; // Create vector to hold our words
      std::stringstream ss(line);
      std::string buf;

      while (ss >> buf) {
        vals.push_back(buf);
      }

      if (vals.size() > 0) 
      {
        std::string tag = vals[0];
        if (tag == "numFunction")
        {
          numFunctions = std::stoi( vals[1]) ;
          tfn_widget.resize(numFunctions);
          tfn_widget_multi.resize(numFunctions);
          m_tfns.resize(numFunctions);
          m_trnfnct_counter = numFunctions+1;
         

        }
        if (tag == "FuncName")
        {
          int index = std::stoi(vals[1])-1;
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
          int index = std::stoi(vals[1])-1;
          std::string points = vals[2];
          size_t pos = 0;
          std::string token;
          std::string delimiter = ";";
          size_t last = 0; size_t next = 0;
          std::string point;
          int poinCounter = 0;
          tfn_widget[index].alpha_control_pts.clear();
          while ((next = points.find(delimiter, last)) != std::string::npos) {

            point = points.substr(last, next - last);
            int comaPos = point.find(",");
            float pointX = std::stof(point.substr(0, comaPos));
            float pointY = std::stof(point.substr(comaPos+1));
            tfn_widget[index].alpha_control_pts.push_back(TransferFunctionWidget::vec2f(pointX, pointY));
            std::cout << point << std::endl;

            last = next + 1;
            poinCounter++;
          }
          point = points.substr(last);
          int comaPos = point.find(",");
          float pointX = std::stof(point.substr(0, comaPos));
          float pointY = std::stof(point.substr(comaPos + 1));
          tfn_widget[index].alpha_control_pts.push_back(TransferFunctionWidget::vec2f(pointX, pointY));
          std::cout << points.substr(last) << std::endl;
          
        }

      }
    }
    loadFile.close();
  }

}

