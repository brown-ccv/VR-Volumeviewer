#include "UI/UIView.h"
#include "UI/UIController.h"
#include "interaction/CreateMovieAction.h"
#include "interaction/HelperFunctions.h"

#include "vrapp/VRVolumeApp.h"

#include <glm/gtc/type_ptr.hpp>

#include "loader/VRDataLoader.h"



UIView::UIView(VRVolumeApp& controllerApp) :m_controller_app(controllerApp), m_multiplier(1.0f), m_threshold(0.0f),
m_z_scale(0.16f), m_scale{ 1.0f }, m_slices(256), m_dynamic_slices(false), m_renderVolume(true), m_selectedTrnFnc(0),
m_animated(false), m_ui_frame_controller(0.0f), m_menu_handler(nullptr), m_initialized(false), m_use_transferfunction(false),
m_clip_max(1.0), m_clip_min(0.0), m_clip_ypr(0.0), m_clip_pos(0.0), m_useCustomClipPlane(false), m_rendermethod(1), m_renderchannel(0),
m_table_selection(-1), m_trn_fct_name_open(false), m_save_trnfct_open(false), m_trnfnct_counter(1), m_file_dialog_open(false),
m_file_load_trnsf(false), m_file_dialog_save_dir(false), m_save_session_dialog_open(false), m_current_save_modal(SAVE_NONE),
m_current_load_modal(LOAD_NONE),m_file_extension_filter(".txt"), m_non_trns_functions_selected_modal(false),
m_ui_background(false)

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

  
  m_ui_background = m_menu_handler->windowIsActive()? true:false;
  

  int flags = ImGuiWindowFlags_NoResize;
  if (!m_ui_background)
  {
    flags |= ImGuiWindowFlags_NoBackground;
  }

  ImGui::Begin("Volumeviewer",NULL, flags);
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

    ImGui::SameLine(ImGui::GetWindowSize().x * 0.5f, 0);
    ImGui::Text("FPS = %f", m_controller_app.get_fps());
    const char* items[] = { "sliced" , "raycast" };
    ImGui::Combo("RenderMethod", &m_rendermethod, items, IM_ARRAYSIZE(items));

    const char* items_channel[] = { "based on data" , "red", "green" , "blue", "alpha", "rgba", "rgba with alpha as max rgb" };
    ImGui::Combo("Render Channel", &m_renderchannel, items_channel, IM_ARRAYSIZE(items_channel));

    ImGui::Checkbox("Render Volume data", &m_renderVolume);

    int numVolumes = m_controller_app.get_num_volumes();


    if (numVolumes > 0) {
      if (ImGui::SmallButton("Save Session")) {
        m_save_session_dialog_open = true;
      }
      ImGui::SameLine();
    }
    
    
    if (ImGui::SmallButton("Load Session")) {
      m_file_load_trnsf = true;
      m_current_load_modal = LOAD_MODAL::LOAD_SESSION;
      m_file_extension_filter = ".usr";
    }

    if (numVolumes > 0)
    {
      if (ImGui::SmallButton("New Function")) {
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
      if (ImGui::SmallButton("Remove Function")) {
        
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

     

      if (ImGui::SmallButton("Clear All Functions")) {
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
      if (ImGui::SmallButton("Save Functions")) {
        m_save_trnfct_open = true;
        m_non_trns_functions_selected_modal = !m_use_transferfunction;
      };

      ImGui::SameLine();
      if (ImGui::SmallButton("Load Functions")) {
        
        m_file_load_trnsf = true;
        m_current_load_modal = LOAD_MODAL::LOAD_TRFR_FNC;
        m_file_extension_filter = ".fnc";
      };

      int selection = 0;
      
      ImGui::BeginTable("##Transfer Function Editor", numVolumes+1, ImGuiTableFlags_Borders);
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
      
      for (int column = 0; column < numVolumes; column++)
      {
        ImGui::TableSetupColumn(m_dataLabels[column].c_str());
      }
      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      for (int column = 1; column < numVolumes+1; column++)
      {
        ImGui::TableSetColumnIndex(column);
        const char* column_name = ImGui::TableGetColumnName(column); 
        ImGui::PushID(column);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::RadioButton("", &m_column_selected, column-1); ImGui::SameLine();
        ImGui::PopStyleVar();
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::TableHeader(column_name);
        ImGui::PopID();
       
      }
      
      
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
                m_trn_fct_name_open = true;
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
      if (m_save_session_dialog_open)
      {
        m_current_save_modal = SAVE_SESSION;
        std::string save_user_session_window_id = "Save User Session";
       
        std::string extension =  ".usr";
        auto save_funtion = std::bind(&UIView::save_user_session, this, std::placeholders::_1);
        open_save_modal_dialog(save_user_session_window_id, m_save_session_dialog_open, save_funtion, extension);
   
      }

      if (m_save_trnfct_open)
      {
        m_current_save_modal = SAVE_TRFR_FNC;
        std::string save_Trnf_window_id = "Save Transfer Functions";
        if (m_use_transferfunction)
        {
          std::string extension = ".fnc";
          auto save_funtion = std::bind(&UIView::save_trans_functions, this, std::placeholders::_1);
          open_save_modal_dialog(save_Trnf_window_id, m_save_trnfct_open, save_funtion, extension);
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
              m_save_trnfct_open = false;
              ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
          }
        }
        
     
      }

      if (m_trn_fct_name_open)
      {
        ImGui::OpenPopup("Change Function Name");
        ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal("Change Function Name", &m_trn_fct_name_open))
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
            m_trn_fct_name_open = false;
            m_current_save_modal = SAVE_MODAL::SAVE_NONE;
            ImGui::CloseCurrentPopup();
          }
          if (ImGui::Button("Cancel"))
          {
            m_trn_fct_name_open = false;
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
            
            tfn_widget[0].setHistogram(m_controller_app.get_volume(m_column_selected)[0]->getTransferfunction(0));
            tfn_widget[0].setMinMax(m_controller_app.get_volume(m_column_selected)[0]->getMin(), m_controller_app.get_volume(0)[0]->getMax());
          }
          m_controller_app.set_multi_transfer(false);
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
  if (m_file_dialog_open)
  {
    ImGui::OpenPopup("Open File");
    m_file_dialog_open = false;
  }

  
   

  if (fileDialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), m_file_extension_filter))
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

  //if (m_save_trnfct_open)
  //{
  //  //ImGui::OpenPopup("Save File");
  //  m_save_trnfct_open = false;
  //}

  if (m_file_dialog_save_dir)
  {
    ImGui::OpenPopup("Save File");
    m_file_dialog_save_dir = false;
    switch (m_current_save_modal)
    {
    case SAVE_SESSION:
      m_save_session_dialog_open = false;
      break;
    case SAVE_TRFR_FNC:
      m_save_trnfct_open = false;
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
    case SAVE_TRFR_FNC:
      m_save_trnfct_open = true;
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
        load_trans_functions(fileToLoad);
        m_table_selection = 0;
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
        m_table_selection = 0;
        m_current_load_modal = LOAD_NONE;
      }
    }
  }

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
    m_menu_handler = new VRMenuHandler(is2D);
    VRMenu* menu = m_menu_handler->addNewMenu(std::bind(&UIView::draw_ui_callback, this), 1024, 1024, 1, 1, fontsize);
    menu->setMenuPose(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 2, -1, 1));
    m_dir_to_save = m_controller_app.get_directory_path().c_str();
    m_initialized = true;

  }


}

void UIView::update_ui(int numVolumes)
{
  tfn_widget_multi.resize(1);
  tfn_widget.resize(1);
  m_use_transferfunction = true;
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

void UIView::render_2D()
{
  if (m_show_menu)
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
  if (m_trn_fct_name_open)
  {
    m_copy_trnfnct_name += c;
  }
  if (m_save_trnfct_open || m_save_session_dialog_open)
  {
    m_save_file_name += c;
  }
}

void UIView::remove_character()
{
  if (m_trn_fct_name_open )
  {
    if (!m_copy_trnfnct_name.empty())
    {
      m_copy_trnfnct_name.pop_back();
    }
    
  }
  
  if (m_save_trnfct_open || m_save_session_dialog_open)
  {
    if (!m_save_file_name.empty())
    {
      m_save_file_name.pop_back();
    }
    
  }
}

void UIView::open_save_modal_dialog(std::string& id, bool& window_state, 
  std::function<void(std::ofstream&)> save_function, std::string& extension)
{
  ImGui::OpenPopup(id.c_str());
  ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
  if (ImGui::BeginPopupModal(id.c_str(), &window_state))
  {
    char* writable = new char[m_save_file_name.size() + 1];
    std::copy(m_save_file_name.begin(), m_save_file_name.end(), writable);
    writable[m_save_file_name.size()] = '\0';
    ImGui::Text(m_dir_to_save.c_str());
    ImGui::SameLine();
    if (ImGui::Button("..."))
    {
      m_file_dialog_save_dir = true;
    }

    ImGui::InputText("##filename", writable, IM_ARRAYSIZE(writable));
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

void UIView::add_trans_function()
{

}

void UIView::save_trans_functions(std::ofstream& saveFile)
{
  std::string pointsLine;
  if (saveFile.is_open())
  {
    saveFile << "numFunction " << std::to_string(m_tfns.size()) << "\n";
    for(int i = 0; i < m_tfns.size();i++)
    {
      //savefile << m_tfns[i].ID + ",";
      saveFile << "FuncName " + std::to_string(i+1) + " " + m_tfns[i].Name + " " + std::to_string(tfn_widget[i].get_colormap_gpu()) +"\n";
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
      saveFile << "FuncPoints " << std::to_string(i+1) << " ";
      for (int j = 0; j < tfn_widget[i].alpha_control_pts.size(); j++)
      {
        if (j != tfn_widget[i].alpha_control_pts.size() - 1)
        {
          saveFile << std::to_string(tfn_widget[i].alpha_control_pts[j].x) + "," + std::to_string(tfn_widget[i].alpha_control_pts[j].y) + ";";
        }
        else
        {
          saveFile << std::to_string(tfn_widget[i].alpha_control_pts[j].x) + "," + std::to_string(tfn_widget[i].alpha_control_pts[j].y) ;
        }

      }
      saveFile << "\n";
    }
    saveFile.close();
  }
}

void UIView::save_user_session(std::ofstream& savefile)
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
    
    if (m_use_transferfunction)
    {
      savefile << "Trnfncs" << "\n";
      save_trans_functions(savefile);
    }

    savefile.close();
  }
}

void UIView::load_trans_functions(std::ifstream& loadFile)
{
  std::string line;
 
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

      while (ss >> buf) {
        vals.push_back(buf);
      }

      std::string tag = vals[0];
      

      if (tag == "volume_loaded")
      {
        std::string fileToLoad = vals[1];
        VRDataLoader::get_instance()->load_txt_file(m_controller_app, fileToLoad);
      }

      else if (tag == "alpha_multiplier") {
        m_multiplier = std::stof(vals[1]);
      }
      else if (tag == "threshold") {
        m_threshold = std::stof(vals[1]);
      }
      else if (tag == "scale") {
        m_scale = std::stof(vals[1]);
      }
      else if (tag == "z-scale") {
        m_z_scale = std::stof(vals[1]);
      }
      else if (tag == "Slices") {
        m_slices = std::stof(vals[1]);
      }
      else if (tag == "automatic _slice adjustment") {
        m_dynamic_slices = std::stof(vals[1]);
      }
      else if (tag == "RenderMethod") {
        m_rendermethod = std::stof(vals[1]);
      }
      else if (tag == "Render_Channel") {
        m_renderchannel = std::stof(vals[1]);
      }
      else if (tag == "Render_Volume_data") {
        m_renderVolume = std::stoi(vals[1]);
      }
      else if (tag == "ClipXmin") {
        m_clip_min.x = std::stof(vals[1]);
      }
      else if (tag == "ClipXmax") {
        m_clip_max.x = std::stof(vals[1]);
      }
      else if (tag == "ClipYmin") {
        m_clip_min.y = std::stof(vals[1]);
      }
      else if (tag == "ClipYmax") {
        m_clip_max.y = std::stof(vals[1]);
      }
      else if (tag == "ClipZmin") {
        m_clip_min.z = std::stof(vals[1]);
      }
      else if (tag == "ClipZmax") {
        m_clip_max.z = std::stof(vals[1]);
      }
      else if (tag == "Trnfncs") 
      {  
        load_trans_functions(loadFile);
      }
    }
    loadFile.close();

  }
}

