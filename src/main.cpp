// If you want to sleep tonight, better not read this sourcecode.
#include "main.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "questui/shared/QuestUI.hpp"

#include "shared/utils/utils-functions.h" // csstrtostr

#include "HMUI/Touchable.hpp"
#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"

#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/HealthWarningViewController.hpp"
#include "GlobalNamespace/SaberModelController.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberTypeObject.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/TimeHelper.hpp"
#include "GlobalNamespace/SaberMovementData.hpp"
#include "GlobalNamespace/ColorManager.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/ColorType.hpp"
#include "GlobalNamespace/GamePause.hpp"

#include "HMUI/CurvedTextMeshPro.hpp"
#include "UnityEngine/GameObject.hpp"
#include "TMPro/TextMeshPro.hpp"
#include "TMPro/TMP_Text.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/PrimitiveType.hpp"

#include "bs-utils/shared/utils.hpp"

#include <sstream>

static ModInfo modInfo {ID, VERSION}; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Controller transforms (includes user set angles and offsets)
UnityEngine::Transform* r_physicalHand = nullptr;
UnityEngine::Transform* l_physicalHand = nullptr;

// For shifting saber forward (shifting at manualUpdate)
UnityEngine::Transform* r_saber_TF = nullptr;
UnityEngine::Transform* l_saber_TF = nullptr;

// Visual handle part of the flail/nunchuck 
UnityEngine::GameObject* r_handleGO = nullptr;
UnityEngine::GameObject* l_handleGO = nullptr;

// Colors
GlobalNamespace::ColorManager* colorManager = nullptr;

struct RopeSegment {
    UnityEngine::Vector3 posNow;
    UnityEngine::Vector3 posOld;
};

std::vector<RopeSegment> r_ropeSegments;
std::vector<RopeSegment> l_ropeSegments;

// Rope segment visualizers
std::vector<UnityEngine::GameObject*> r_ropeVisuals;
std::vector<UnityEngine::GameObject*> l_ropeVisuals;

UnityEngine::Material* r_visualMaterial = nullptr;
UnityEngine::Material* l_visualMaterial = nullptr;

// Initialized when settings tab is opened
QuestUI::IncrementSetting *ui_handleLength         = nullptr;
QuestUI::IncrementSetting *ui_flailLength          = nullptr;
QuestUI::IncrementSetting *ui_ropeSegLen           = nullptr;
QuestUI::IncrementSetting *ui_lastRopeSegLen       = nullptr;
QuestUI::IncrementSetting *ui_segmentLength        = nullptr;
QuestUI::IncrementSetting *ui_forceGravity         = nullptr;
QuestUI::IncrementSetting *ui_applyConstraintCount = nullptr;
QuestUI::IncrementSetting *ui_applyAmount          = nullptr;
QuestUI::IncrementSetting *ui_velocityMultiplier   = nullptr;

// ------ Settings 1 ------
//float handleLength = 0.24f;
//float flailLength  = 0.36f;
//float shiftFlailForward = 0.13f;
//
//float ropeSegLen = 0.015f;
//float lastRopeSegLen = 0.16f; // LONGER LAST NODE SIMULATES WEIGHT
//
//int segmentLength = 9;
//UnityEngine::Vector3 forceGravity{0.0f, -0.13f, 0.0f};
//int applyConstraintCount = 9;
//float applyAmount = 0.490f; // 0.01f - 1.0f
//float velocityMultiplier = 0.94f;

// ------ Settings 2 ------
//float handleLength = 0.19f;
//float flailLength  = 0.36f;
//float shiftFlailForward = 0.15f;
//
//float ropeSegLen = 0.016f;
//float lastRopeSegLen = 0.22f; // LONGER LAST NODE SIMULATES WEIGHT
//
//int segmentLength = 4;
//UnityEngine::Vector3 forceGravity{0.0f, -0.22f, 0.0f};
//int applyConstraintCount = 9;
//float applyAmount = 0.490f; // 0.01f - 1.0f
//float velocityMultiplier = 0.98f;

// ------ Settings 3 ------
//float handleLength = 0.29f;
//float flailLength  = 0.36f;
//float shiftFlailForward = 0.15f;
//
//float ropeSegLen = 0.012f;
//float lastRopeSegLen = 0.22f; // LONGER LAST NODE SIMULATES WEIGHT
//
//int segmentLength = 10;
//UnityEngine::Vector3 forceGravity{0.0f, -0.18f, 0.0f};
//int applyConstraintCount = 14;
//float applyAmount = 0.485f; // 0.01f - 1.0f
//float velocityMultiplier = 0.97f;

// ------ Settings 4 ------
//float handleLength = 0.32f;
//float flailLength  = 0.36f;
//float shiftFlailForward = 0.22f;
//
//float ropeSegLen = 0.021f;
//float lastRopeSegLen = 0.215f; // LONGER LAST NODE SIMULATES WEIGHT
//
//int segmentLength = 4;
//UnityEngine::Vector3 forceGravity{0.0f, -0.19f, 0.0f};
//int applyConstraintCount = 32;
//float applyAmount = 0.485f; // 0.01f - 1.0f
//float velocityMultiplier = 0.982f;

// ------ Settings 0 (Long) ------
const float default0_handleLength = 0.32f;
const float default0_flailLength  = 0.36f;
const float default0_ropeSegLen = 0.04f;
const float default0_lastRopeSegLen = 0.09f; // LONGER LAST NODE SIMULATES WEIGHT
const int   default0_segmentLength = 13;
const float default0_forceGravity = -0.25f;
const int   default0_applyConstraintCount = 30;
const float default0_applyAmount = 0.485f; // 0.01f - 1.0f
const float default0_velocityMultiplier = 0.982f;

// ------ Settings 1 (Short) ------
const float default1_handleLength = 0.2f;
const float default1_flailLength  = 0.4f;
const int   default1_segmentLength = 4;
const float default1_ropeSegLen = 0.03f;
const float default1_lastRopeSegLen = 0.2f; // LONGER LAST NODE SIMULATES WEIGHT
const float default1_forceGravity = -0.25f;
const int   default1_applyConstraintCount = 15;
const float default1_applyAmount = 0.40f; // 0.01f - 1.0f
const float default1_velocityMultiplier = 0.97f;

// ------ Settings 2 (SomePreset) ------
const float default2_handleLength         = 0.32f;
const float default2_flailLength          = 0.36f;
const float default2_ropeSegLen           = 0.021f;
const float default2_lastRopeSegLen       = 0.215f; 
const int   default2_segmentLength        = 4;
const float default2_forceGravity         = -0.19f;
const int   default2_applyConstraintCount = 32;
const float default2_applyAmount          = 0.485f; 
const float default2_velocityMultiplier   = 0.982f;

struct{
    bool flailSaber_enabled    = true;
    float handleLength         = default1_handleLength;        
    float flailLength          = default1_flailLength;         
    float ropeSegLen           = default1_ropeSegLen;          
    float lastRopeSegLen       = default1_lastRopeSegLen;      
    int   segmentLength        = default1_segmentLength;       
    float forceGravity         = default1_forceGravity;        
    int   applyConstraintCount = default1_applyConstraintCount;
    float applyAmount          = default1_applyAmount;         
    float velocityMultiplier   = default1_velocityMultiplier;  
} flail_config;


void _UPDATE_SCORE_SUBMISSION(){
    if(flail_config.flailSaber_enabled)
        bs_utils::Submission::disable(modInfo);
    else
        bs_utils::Submission::enable(modInfo);
}

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

void SaveConfig() {
    getConfig().config.RemoveAllMembers();
    getConfig().config.SetObject();
    rapidjson::Document::AllocatorType& allocator = getConfig().config.GetAllocator();

    getConfig().config.AddMember("flailSaber_enabled",  flail_config.flailSaber_enabled, allocator);

    getConfig().config.AddMember("handleLength",        flail_config.handleLength, allocator);
    getConfig().config.AddMember("flailLength",         flail_config.flailLength, allocator);
    getConfig().config.AddMember("ropeSegLen",          flail_config.ropeSegLen, allocator);
    getConfig().config.AddMember("lastRopeSegLen",      flail_config.lastRopeSegLen, allocator);
    getConfig().config.AddMember("segmentLength",       flail_config.segmentLength, allocator);
    getConfig().config.AddMember("forceGravity",        flail_config.forceGravity, allocator);
    getConfig().config.AddMember("applyConstraintCount",flail_config.applyConstraintCount, allocator);
    getConfig().config.AddMember("applyAmount",         flail_config.applyAmount, allocator);
    getConfig().config.AddMember("velocityMultiplier",  flail_config.velocityMultiplier, allocator);
    
    getConfig().Write();
    getLogger().info("Configuration Saved!");
}

bool _CONFIG_TRY_GET_FLOAT(const char* jsonName, float* config_item_refrence){
    bool itemExists = getConfig().config.HasMember(jsonName) && getConfig().config[jsonName].IsFloat();
    if(itemExists) *config_item_refrence = getConfig().config[jsonName].GetFloat();
    return itemExists;
}
bool _CONFIG_TRY_GET_INT(const char* jsonName, int* config_item_refrence){
    bool itemExists = getConfig().config.HasMember(jsonName) && getConfig().config[jsonName].IsInt();
    if(itemExists) *config_item_refrence = getConfig().config[jsonName].GetInt();
    return itemExists;
}

bool LoadConfig() {
    getLogger().info("Loading Configuration...");
    getConfig().Load();
    bool foundEverything = true;


    bool itemExists = getConfig().config.HasMember("flailSaber_enabled") && getConfig().config["flailSaber_enabled"].IsBool();
    if(itemExists) flail_config.flailSaber_enabled = getConfig().config["flailSaber_enabled"].GetBool();
    else           foundEverything = false;

    bool itemCheck; 
    itemCheck = _CONFIG_TRY_GET_FLOAT("handleLength", &(flail_config.handleLength) );
    if(!itemCheck) foundEverything = false;
    itemCheck = _CONFIG_TRY_GET_FLOAT("flailLength",  &(flail_config.flailLength) );
    if(!itemCheck) foundEverything = false;
    itemCheck = _CONFIG_TRY_GET_FLOAT("ropeSegLen", &(flail_config.ropeSegLen) );
    if(!itemCheck) foundEverything = false;
    itemCheck = _CONFIG_TRY_GET_FLOAT("lastRopeSegLen", &(flail_config.lastRopeSegLen) );
    if(!itemCheck) foundEverything = false;

    itemCheck = _CONFIG_TRY_GET_INT("segmentLength", &(flail_config.segmentLength) );
    if(!itemCheck) foundEverything = false;
    itemCheck = _CONFIG_TRY_GET_FLOAT("forceGravity", &(flail_config.forceGravity) );
    if(!itemCheck) foundEverything = false;
    itemCheck = _CONFIG_TRY_GET_INT("applyConstraintCount", &(flail_config.applyConstraintCount) );
    if(!itemCheck) foundEverything = false;
    itemCheck = _CONFIG_TRY_GET_FLOAT("applyAmount", &(flail_config.applyAmount) );
    if(!itemCheck) foundEverything = false;
    itemCheck = _CONFIG_TRY_GET_FLOAT("velocityMultiplier", &(flail_config.velocityMultiplier) );
    if(!itemCheck) foundEverything = false;

    if (foundEverything)
        getLogger().info("Configuration Loaded Successfully!");
    else
        getLogger().info("Configuration Failed To Load Properly!");

    return foundEverything;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}


template <typename T>
void LOG_ALL_THINGS_UNDER_GAMEOBJECT(UnityEngine::GameObject* gameobj, std::string prependStr=""){
    auto componentArr = gameobj->GetComponents<T>();
    std::stringstream buff;
    for (int i=0; i<componentArr->Length(); i++){
        auto comp = (*componentArr)[i];
        auto name = comp->get_name();
        auto fullName = comp->GetType()->get_FullName();
        buff << prependStr.c_str() << to_utf8(csstrtostr(fullName)).c_str() << " " << to_utf8(csstrtostr(name)).c_str() << "\n";
    }
    getLogger().info("%s",buff.str().c_str());
}
template <typename T>
void LOG_ALL_THINGS_UNDER_GAMEOBJECT_CHILDREN(UnityEngine::GameObject* gameobj, std::string prependStr=""){
    auto componentArr = gameobj->GetComponentsInChildren<T>();
    std::stringstream buff;
    for (int i=0; i<componentArr->Length(); i++){
        auto comp = (*componentArr)[i];
        auto name = comp->get_name();
        auto fullName = comp->GetType()->get_FullName();
        buff << prependStr.c_str() << to_utf8(csstrtostr(fullName)).c_str() << " " << to_utf8(csstrtostr(name)).c_str() << "\n";
    }
    getLogger().info("%s",buff.str().c_str());
}


// Hook 2.5 Colors
MAKE_HOOK_MATCH(
    ColorManager_ColorForSaberType, 
    &GlobalNamespace::ColorManager::ColorForSaberType,
    UnityEngine::Color,
    GlobalNamespace::ColorManager* self, 
    GlobalNamespace::SaberType type
) {
    
    auto ret = ColorManager_ColorForSaberType(self, type);

    // Set the color manager
    colorManager = self;

    return ret;
}

void _INITIALIZE_FLAIL_SABER(GlobalNamespace::Saber* saber){
    bool rightsaber_bool = true;
    if (saber->get_saberType() == GlobalNamespace::SaberType::SaberA){
        rightsaber_bool = false;
    } 

    // BREAK WHEN NOT IN GAME
    if(rightsaber_bool == true){
        if(to_utf8(csstrtostr(  saber->get_transform()->get_name() )).compare("RightSaber") != 0){
            return;
        }
    }
    else{
        if(to_utf8(csstrtostr(  saber->get_transform()->get_name() )).compare("LeftSaber") != 0){
            return;
        }
    }
    
    // Poor usage of if statements, but its only Init so who cares lol
    if (rightsaber_bool) r_saber_TF = saber->get_transform();
    else                 l_saber_TF = saber->get_transform();
    
    UnityEngine::Transform* _saberBottom = nullptr;
    UnityEngine::Transform* _saberTop = nullptr;
    UnityEngine::GameObject* _customHandle = nullptr;


    getLogger().info("L or R Saber initialized");

    // ####### ------- ####### ------- ####### ------- #######

    // "Start()"
    UnityEngine::Vector3 ropeStartPoint{0.58f, 1.4f, 0.2f};
    
    // IMPORTANT TO CLEAR! Otherwise vector gets bigger every song.
    // Without clear it works just as well but memory is lost..
    r_ropeSegments.clear(); 
    l_ropeSegments.clear(); 

    // Clean rope visuals
    r_ropeVisuals.clear();
    l_ropeVisuals.clear();
    r_visualMaterial = nullptr;
    l_visualMaterial = nullptr;

    for (int i = 0; i < flail_config.segmentLength; i++) {
        RopeSegment newsegment{
            ropeStartPoint,
            ropeStartPoint
        };

        // Create new visualisation sphere
        UnityEngine::GameObject* sphereGO = UnityEngine::GameObject::CreatePrimitive(UnityEngine::PrimitiveType::Sphere); // Cube = 3
        sphereGO->set_name(il2cpp_utils::createcsstr("sphereVisuals"));
        sphereGO->get_transform()->SetParent(saber->get_gameObject()->get_transform()->get_parent()->get_parent(), false);
        sphereGO->get_transform()->set_localPosition(UnityEngine::Vector3{0,0,0});
        sphereGO->get_transform()->set_localScale(UnityEngine::Vector3(0.04f, 0.04f, 0.04f));
        sphereGO->get_transform()->set_position(ropeStartPoint);

        // Add rope visuals to vector
        if(rightsaber_bool){
            std::vector<RopeSegment>::iterator it_r = r_ropeSegments.begin();
            r_ropeSegments.insert(it_r, newsegment);
            r_ropeVisuals.push_back(sphereGO);
        }
        else{
            std::vector<RopeSegment>::iterator it_l = l_ropeSegments.begin();
            l_ropeSegments.insert(it_l, newsegment);
            l_ropeVisuals.push_back(sphereGO);
        }
        
        if (i == flail_config.segmentLength - 2)
            ropeStartPoint.y -= flail_config.lastRopeSegLen;
        else
            ropeStartPoint.y -= flail_config.ropeSegLen;
        
    }

    _saberBottom  = saber->get_gameObject()->get_transform()->Find(il2cpp_utils::newcsstr("Bottom"));
    _saberTop     = saber->get_gameObject()->get_transform()->Find(il2cpp_utils::newcsstr("Top"));
    if (rightsaber_bool == true){
        r_physicalHand = saber->get_gameObject()->get_transform()->get_parent();
        // Take saber away from hand
        saber->get_gameObject()->get_transform()->set_parent(r_physicalHand->get_parent());
    }
    else{
        l_physicalHand = saber->get_gameObject()->get_transform()->get_parent();
        // Take saber away from hand
        saber->get_gameObject()->get_transform()->set_parent(l_physicalHand->get_parent());
    }

    // Scale this down to look more like nunchucks
    saber->get_transform()->set_localScale(UnityEngine::Vector3{1.0f,1.0f,flail_config.flailLength});

    // Handle (created from saber GameObject)
    _customHandle = UnityEngine::GameObject::Instantiate(saber->get_gameObject());

    // Removing all unneccecary components from handle part
    // And Changing its color/material of different parts
    auto componentArr = _customHandle->GetComponents<UnityEngine::Component*>();
    for (int i=0; i<componentArr->Length(); i++){
        auto comp = (*componentArr)[i];
        auto fullName = comp->GetType()->get_FullName();
        int compare = fullName->CompareTo(il2cpp_utils::createcsstr("SaberModelContainer"));
        if (compare != 0){
            UnityEngine::Component::Destroy(comp);
        }
        else{

            // Remove All COMPONENTS excepty Transforms, and MeshFilters/MeshRenderers
            auto componentArr2 = comp->get_gameObject()->GetComponentsInChildren<UnityEngine::Component*>();
            UnityEngine::Material* material = nullptr;
            for (int i=0; i<componentArr2->Length(); i++){
                UnityEngine::Component* comp2 = (*componentArr2)[i];
                Il2CppString* name2 = comp2->get_name();
                Il2CppString* fullName2 = comp2->GetType()->get_FullName();
                bool isMeshRenderer = fullName2->Contains(il2cpp_utils::createcsstr("MeshRenderer"));
                bool orCompare = fullName2->Contains(il2cpp_utils::createcsstr("Transform")) ||
                                    fullName2->Contains(il2cpp_utils::createcsstr("MeshFilter")) ||
                                    isMeshRenderer ;
                if( !orCompare ){
                    // DESTROYING ALL COMPONENTS EXECPT THE ONES MENTIONED ABOVE
                    UnityEngine::Component::Destroy(comp2);
                }

                if (isMeshRenderer){
                    if(rightsaber_bool){
                        if (r_visualMaterial == nullptr){
                            UnityEngine::Color color;
                            color = colorManager->ColorForSaberType(GlobalNamespace::SaberType::SaberB);
                            UnityEngine::Color saberColor(color.r, color.g, color.b, 0.0f);
                            material = UnityEngine::Material::New_ctor( ((UnityEngine::MeshRenderer*)comp2)->get_material() );
                            material->SetColor(il2cpp_utils::createcsstr("_Color"), saberColor);
                            r_visualMaterial = material;
                        }
                    } else {
                        if (l_visualMaterial == nullptr){
                            UnityEngine::Color color;
                            color = colorManager->ColorForSaberType(GlobalNamespace::SaberType::SaberA);
                            UnityEngine::Color saberColor(color.r, color.g, color.b, 0.0f);
                            material = UnityEngine::Material::New_ctor( ((UnityEngine::MeshRenderer*)comp2)->get_material() );
                            material->SetColor(il2cpp_utils::createcsstr("_Color"), saberColor);
                            l_visualMaterial = material;
                        }
                    }

                    ((UnityEngine::MeshRenderer*)comp2)->set_material(material);

                }
            }
        }
    }

    // Resizing the handle and setting its parent to hieracially correct place
    _customHandle->get_transform()->set_localScale(UnityEngine::Vector3{1,1,flail_config.handleLength});
    if (rightsaber_bool == true){
        _customHandle->get_transform()->set_parent(r_physicalHand);
        r_handleGO = _customHandle;
        for(int k=0; k<flail_config.segmentLength; k++){
            r_ropeVisuals[k]->GetComponent<UnityEngine::MeshRenderer*>()->set_material(r_visualMaterial);
        }
    }
    else{
        _customHandle->get_transform()->set_parent(l_physicalHand);
        l_handleGO = _customHandle;
        for(int k=0; k<flail_config.segmentLength; k++){
            l_ropeVisuals[k]->GetComponent<UnityEngine::MeshRenderer*>()->set_material(l_visualMaterial);
        }
    }
    _customHandle->get_transform()->set_localPosition(UnityEngine::Vector3{0,0,0});
}

// Hook 3
MAKE_HOOK_MATCH(
    SaberModelController_Init, 
    &GlobalNamespace::SaberModelController::Init,
    void,
    GlobalNamespace::SaberModelController* self, 
    UnityEngine::Transform* parent, 
    GlobalNamespace::Saber* saber
) {

    getLogger().info("SaberModelController::Init() Called.");

    SaberModelController_Init(self, parent, saber);

    if(flail_config.flailSaber_enabled) _INITIALIZE_FLAIL_SABER(saber);
}

// Hook 4 Game Pause
MAKE_HOOK_MATCH(
    GamePause_Pause, 
    &GlobalNamespace::GamePause::Pause,
    void,
    GlobalNamespace::GamePause* self
) {
    GamePause_Pause(self);

    if(flail_config.flailSaber_enabled){
        r_handleGO->set_active(false);
        l_handleGO->set_active(false);

        for(int i=0; i<flail_config.segmentLength; i++) {
            r_ropeVisuals[i]->set_active(false);
            l_ropeVisuals[i]->set_active(false);
        }
    }
}
// Hook 5 Game Resume
MAKE_HOOK_MATCH(
    GamePause_WillResume, 
    &GlobalNamespace::GamePause::WillResume,
    void,
    GlobalNamespace::GamePause* self
) {
    GamePause_WillResume(self);

    if(flail_config.flailSaber_enabled){
        r_handleGO->set_active(true);
        l_handleGO->set_active(true);

        for(int i=0; i<flail_config.segmentLength; i++) {
            r_ropeVisuals[i]->set_active(true);
            l_ropeVisuals[i]->set_active(true);
        }
    }
}


void ApplyConstraint(std::__ndk1::vector<RopeSegment>& _ropeSegments, UnityEngine::Transform* _physicalHand){

    RopeSegment firstSegment = _ropeSegments[0];
    
    firstSegment.posNow = _physicalHand->get_position() + (_physicalHand->get_forward()*flail_config.handleLength);
    
    _ropeSegments[0] = firstSegment;

    for (int i = 0; i < flail_config.segmentLength - 1; i++)
    {
        RopeSegment firstSeg  = _ropeSegments[i];
        RopeSegment secondSeg = _ropeSegments[i + 1];
 
        float dist = (firstSeg.posNow - secondSeg.posNow).get_magnitude();
        float error = dist;
        if(i == flail_config.segmentLength -2) 
            error -= flail_config.lastRopeSegLen;
        else
            error -= flail_config.ropeSegLen;

        UnityEngine::Vector3 changeDir = (firstSeg.posNow - secondSeg.posNow).get_normalized();
        UnityEngine::Vector3 changeAmount = changeDir * error;

        if (i != 0)
        {
            firstSeg.posNow = firstSeg.posNow - changeAmount * (1.0f - flail_config.applyAmount);
            _ropeSegments[i] = firstSeg;
            secondSeg.posNow = secondSeg.posNow + changeAmount * (flail_config.applyAmount);
            _ropeSegments[i + 1] = secondSeg;
        }
        else
        {
            secondSeg.posNow = secondSeg.posNow + changeAmount;
            _ropeSegments[i + 1] = secondSeg;
        }
    }
}

void Simulate(std::__ndk1::vector<RopeSegment>& _ropeSegments, UnityEngine::Transform* _physicalHand) {
    // Verlet integration, Rope-simulation

    // SIMULATION
    float deltaTime = GlobalNamespace::TimeHelper::get_deltaTime();

    for (int i = 1; i < flail_config.segmentLength; i++) {
        RopeSegment firstSegment = _ropeSegments[i];
        UnityEngine::Vector3 velocity = firstSegment.posNow - firstSegment.posOld;
        firstSegment.posOld = firstSegment.posNow;
        firstSegment.posNow = firstSegment.posNow + velocity*flail_config.velocityMultiplier;
        firstSegment.posNow = firstSegment.posNow + UnityEngine::Vector3(0, flail_config.forceGravity * deltaTime, 0);
        _ropeSegments[i] = firstSegment;
    }  

    //CONSTRAINTS
    for (int i = 0; i < flail_config.applyConstraintCount; i++)
    {
        ApplyConstraint(_ropeSegments, _physicalHand);
    }
}

MAKE_HOOK_MATCH(
    Saber_ManualUpdate, 
    &GlobalNamespace::Saber::ManualUpdate,
    void,
    GlobalNamespace::Saber* self
) {

    if(flail_config.flailSaber_enabled){
        if(self->get_saberType() == GlobalNamespace::SaberType::SaberB){
            // "Update()"
            Simulate(r_ropeSegments, r_physicalHand);

            // Set rope visualizer position
            for (int i = 1; i < flail_config.segmentLength; i++) {
                r_ropeVisuals[i]->get_transform()->set_position( r_ropeSegments[i].posNow);
            }  

            // Set saber position
            RopeSegment lastSeg          = r_ropeSegments[flail_config.segmentLength - 1];
            RopeSegment oneBeforeLastSeg = r_ropeSegments[flail_config.segmentLength - 2];
            
            //UnityEngine::Vector3 targetPos = oneBeforeLastSeg.posNow + r_saber_TF->get_forward()*shiftFlailForward;
            UnityEngine::Vector3 targetPos = oneBeforeLastSeg.posNow + r_saber_TF->get_forward()*flail_config.flailLength/(float)4;
            UnityEngine::Vector3 targetDirAsVec = (lastSeg.posNow - oneBeforeLastSeg.posNow);
            
            self->get_transform()->set_position(targetPos);
            self->get_transform()->set_rotation(UnityEngine::Quaternion::LookRotation(targetDirAsVec));
        }
        else{
            // "Update()"
            Simulate(l_ropeSegments, l_physicalHand);

            // Set rope visualizer position
            for (int i = 1; i < flail_config.segmentLength; i++) {
                l_ropeVisuals[i]->get_transform()->set_position( l_ropeSegments[i].posNow);
            }  

            RopeSegment lastSeg          = l_ropeSegments[flail_config.segmentLength - 1];
            RopeSegment oneBeforeLastSeg = l_ropeSegments[flail_config.segmentLength - 2];
            
            //UnityEngine::Vector3 targetPos = oneBeforeLastSeg.posNow + l_saber_TF->get_forward()*shiftFlailForward;
            UnityEngine::Vector3 targetPos = oneBeforeLastSeg.posNow + l_saber_TF->get_forward()*flail_config.flailLength/(float)4;
            UnityEngine::Vector3 targetDirAsVec = (lastSeg.posNow - oneBeforeLastSeg.posNow);
            
            self->get_transform()->set_position(targetPos);
            self->get_transform()->set_rotation(UnityEngine::Quaternion::LookRotation(targetDirAsVec));

        }
    }
    Saber_ManualUpdate(self);
}

void _RESET_CONFIG_VALUES(int reset_type){
    float default_handleLength;        
    float default_flailLength;         
    float default_ropeSegLen;          
    float default_lastRopeSegLen;      
    int   default_segmentLength;       
    float default_forceGravity;        
    int   default_applyConstraintCount;
    float default_applyAmount;         
    float default_velocityMultiplier;  

    if(reset_type == 0){
        default_handleLength         = default0_handleLength;        
        default_flailLength          = default0_flailLength;         
        default_ropeSegLen           = default0_ropeSegLen;          
        default_lastRopeSegLen       = default0_lastRopeSegLen;      
        default_segmentLength        = default0_segmentLength;       
        default_forceGravity         = default0_forceGravity;        
        default_applyConstraintCount = default0_applyConstraintCount;
        default_applyAmount          = default0_applyAmount;         
        default_velocityMultiplier   = default0_velocityMultiplier;  
    } else if(reset_type == 1){
        default_handleLength         = default1_handleLength;        
        default_flailLength          = default1_flailLength;         
        default_ropeSegLen           = default1_ropeSegLen;          
        default_lastRopeSegLen       = default1_lastRopeSegLen;      
        default_segmentLength        = default1_segmentLength;       
        default_forceGravity         = default1_forceGravity;        
        default_applyConstraintCount = default1_applyConstraintCount;
        default_applyAmount          = default1_applyAmount;         
        default_velocityMultiplier   = default1_velocityMultiplier;  
    } else if(reset_type == 2){
        default_handleLength         = default2_handleLength;        
        default_flailLength          = default2_flailLength;         
        default_ropeSegLen           = default2_ropeSegLen;          
        default_lastRopeSegLen       = default2_lastRopeSegLen;      
        default_segmentLength        = default2_segmentLength;       
        default_forceGravity         = default2_forceGravity;        
        default_applyConstraintCount = default2_applyConstraintCount;
        default_applyAmount          = default2_applyAmount;         
        default_velocityMultiplier   = default2_velocityMultiplier;  
    }

    flail_config.handleLength         = default_handleLength        ;
    flail_config.flailLength          = default_flailLength         ;
    flail_config.ropeSegLen           = default_ropeSegLen          ;
    flail_config.lastRopeSegLen       = default_lastRopeSegLen      ;
    flail_config.segmentLength        = default_segmentLength       ;
    flail_config.forceGravity         = default_forceGravity        ;
    flail_config.applyConstraintCount = default_applyConstraintCount;
    flail_config.applyAmount          = default_applyAmount         ;
    flail_config.velocityMultiplier   = default_velocityMultiplier  ;
  
    ui_handleLength->CurrentValue         = default_handleLength            ;
    ui_flailLength->CurrentValue          = default_flailLength             ;
    ui_ropeSegLen->CurrentValue           = default_ropeSegLen              ;
    ui_lastRopeSegLen->CurrentValue       = default_lastRopeSegLen          ;
    ui_segmentLength->CurrentValue        = default_segmentLength           ;
    ui_forceGravity->CurrentValue         = default_forceGravity            ;
    ui_applyConstraintCount->CurrentValue = default_applyConstraintCount    ;
    ui_applyAmount->CurrentValue          = 0.5 - default_applyAmount       ; // NOTE THAT VISUAL VALUE DIFFERES FROM ACTUAL VALUE
    ui_velocityMultiplier->CurrentValue   = 1.0 - default_velocityMultiplier; // NOTE THAT VISUAL VALUE DIFFERES FROM ACTUAL VALUE

    ui_handleLength->UpdateValue();
    ui_flailLength->UpdateValue();         
    ui_ropeSegLen->UpdateValue();          
    ui_lastRopeSegLen->UpdateValue();      
    ui_segmentLength->UpdateValue();       
    ui_forceGravity->UpdateValue();        
    ui_applyConstraintCount->UpdateValue();
    ui_applyAmount->UpdateValue();         
    ui_velocityMultiplier->UpdateValue();  
}

void QuestUI_DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){

    if(firstActivation){
        getLogger().info("QuestUI First Activation !");
        self->get_gameObject()->AddComponent<HMUI::Touchable*>();

        UnityEngine::GameObject* container = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());
        UnityEngine::Transform*  parent = container->get_transform();
    
        QuestUI::BeatSaberUI::CreateToggle( parent, "Mod Enabled",  flail_config.flailSaber_enabled, [](bool value) -> void { 
            flail_config.flailSaber_enabled = value;
            _UPDATE_SCORE_SUBMISSION();
            SaveConfig();
        });

       
        QuestUI::BeatSaberUI::CreateUIButton(parent, "Preset (Normal Rope/Chain)",  []() -> void { 
            _RESET_CONFIG_VALUES(2);
        });
        QuestUI::BeatSaberUI::CreateUIButton(parent, "Preset (Long  Rope/Chain)",  []() -> void { 
            _RESET_CONFIG_VALUES(0);
        });

        ui_handleLength = QuestUI::BeatSaberUI::CreateIncrementSetting( parent, "Handle length (m)",  3,  0.025, flail_config.handleLength, 0.1, 0.5, [](float value) -> void { 
            flail_config.handleLength = value;
            SaveConfig();
        });
        
        ui_flailLength = QuestUI::BeatSaberUI::CreateIncrementSetting( parent, "Saber/flail length (m)",   3,  0.025, flail_config.flailLength, 0.1, 0.5, [](float value) -> void { 
            flail_config.flailLength = value;
            SaveConfig();
        });

        // -- "Wrong place" compared to struct
        ui_segmentLength = QuestUI::BeatSaberUI::CreateIncrementSetting( parent, "Rope/Chain Segment count",   0,  1, flail_config.segmentLength, 2, 20, [](int value) -> void { 
            flail_config.segmentLength = value;
            SaveConfig();
        });
        // -- "Wrong place" compared to struct

        ui_ropeSegLen = QuestUI::BeatSaberUI::CreateIncrementSetting( parent, "Rope/Chain Segment distance (m)",   3,  0.01, flail_config.ropeSegLen, 0.01, 0.15, [](float value) -> void { 
            flail_config.ropeSegLen = value;
            SaveConfig();
        });
        ui_lastRopeSegLen = QuestUI::BeatSaberUI::CreateIncrementSetting( parent, "Last segment dist(simulates saber weight)",   3,  0.02, flail_config.lastRopeSegLen, 0.02, 0.30, [](float value) -> void { 
            flail_config.lastRopeSegLen = value;
            SaveConfig();
        });

        ui_forceGravity = QuestUI::BeatSaberUI::CreateIncrementSetting( parent, "Gravity force (can be positive)", 3,  0.05, flail_config.forceGravity, -0.4, 0.4, [](float value) -> void { 
            flail_config.forceGravity = value;
            SaveConfig();
        });
        ui_applyConstraintCount = QuestUI::BeatSaberUI::CreateIncrementSetting( parent, "Physics apply count (smaller -> stretchier)",   0,  1, flail_config.applyConstraintCount, 1, 30, [](int value) -> void { 
            flail_config.applyConstraintCount = value;
            SaveConfig();
        });
        ui_applyAmount = QuestUI::BeatSaberUI::CreateIncrementSetting( parent, "Stretchiness", 2,  0.025, flail_config.applyAmount, 0.0, 0.15, [](float value) -> void { 
            flail_config.applyAmount = 0.5 - value;
            SaveConfig();
        });
        ui_velocityMultiplier = QuestUI::BeatSaberUI::CreateIncrementSetting( parent, "Static Friction", 2,  0.01, flail_config.velocityMultiplier, 0.0, 0.1, [](float value) -> void { 
            flail_config.velocityMultiplier = 1.0 - value;
            SaveConfig();
        });

    }
    getLogger().info("Activation activated");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {

    il2cpp_functions::Init();
    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, QuestUI_DidActivate);
    
    if(!LoadConfig())
        SaveConfig();

    _UPDATE_SCORE_SUBMISSION();

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(getLogger(), SaberModelController_Init);
    INSTALL_HOOK(getLogger(), Saber_ManualUpdate);
    INSTALL_HOOK(getLogger(), ColorManager_ColorForSaberType);
    INSTALL_HOOK(getLogger(), GamePause_WillResume);
    INSTALL_HOOK(getLogger(), GamePause_Pause);    
    getLogger().info("Installed all hooks!");

    
}

