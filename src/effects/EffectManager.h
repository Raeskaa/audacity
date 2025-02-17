/**********************************************************************

  Audacity: A Digital Audio Editor

  EffectManager.h

  Audacity(R) is copyright (c) 1999-2008 Audacity Team.
  License: GPL v2 or later.  See License.txt.

**********************************************************************/

#ifndef __AUDACITY_EFFECTMANAGER__
#define __AUDACITY_EFFECTMANAGER__

#include <memory>
#include <vector>

#include <unordered_map>
#include "EffectInterface.h"
#include "EffectHostInterface.h" // for EffectDialogFactory
#include "Identifier.h"

class AudacityCommand;
class AudacityProject;
class CommandContext;
class CommandMessageTarget;
class ComponentInterfaceSymbol;
class TrackList;
class SelectedRegion;
class wxString;
typedef wxString PluginID;

using EffectMap = std::unordered_map<wxString, EffectUIHostInterface *>;
using AudacityCommandMap = std::unordered_map<wxString, AudacityCommand *>;
using EffectOwnerMap = std::unordered_map< wxString, std::shared_ptr<EffectUIHostInterface> >;

class AudacityCommand;


class AUDACITY_DLL_API EffectManager
{
public:

   enum : unsigned {
      // No flags specified
      kNone = 0x00,
      // Flag used to disable prompting for configuration parameteres.
      kConfigured = 0x01,
      // Flag used to disable saving the state after processing.
      kSkipState = 0x02,
      // Flag used to disable "Repeat Last Effect"
      kDontRepeatLast = 0x04,
      // Flag used to disable "Select All during Repeat Generator Effect"
      kRepeatGen = 0x08,
      // Flag used for repeating Nyquist Prompt
      kRepeatNyquistPrompt = 0x10,
   };

   /*! Create a new instance of an effect by its ID. */
   static std::unique_ptr<EffectProcessor> NewEffect(const PluginID &ID);

   /** Get the singleton instance of the EffectManager. Probably not safe
       for multi-thread use. */
   static EffectManager & Get();

//
// public methods
//
// Used by the outside program to register the list of effects and retrieve
// them by index number, usually when the user selects one from a menu.
//
public:
   EffectManager();
   virtual ~EffectManager();

   //! Here solely for the purpose of Nyquist Workbench until a better solution is devised.
   /** Register an effect so it can be executed.
     uEffect is expected to be a self-hosting Nyquist effect */
   const PluginID & RegisterEffect(std::unique_ptr<EffectUIHostInterface> uEffect);
   //! Used only by Nyquist Workbench module
   void UnregisterEffect(const PluginID & ID);

   TranslatableString GetEffectFamilyName(const PluginID & ID);
   TranslatableString GetVendorName(const PluginID & ID);

   /** Run a command given the plugin ID */
   // Returns true on success. 
   bool DoAudacityCommand(const PluginID & ID,
                         const CommandContext &,
                         wxWindow *parent,
                         bool shouldPrompt  = true );

   // Renamed from 'Effect' to 'Command' prior to moving out of this class.
   ComponentInterfaceSymbol GetCommandSymbol(const PluginID & ID);
   TranslatableString GetCommandName(const PluginID & ID);
   CommandID GetCommandIdentifier(const PluginID & ID);
   TranslatableString GetCommandDescription(const PluginID & ID);
   ManualPageID GetCommandUrl(const PluginID & ID);
   TranslatableString GetCommandTip(const PluginID & ID);
   // flags control which commands are included.
   void GetCommandDefinition(const PluginID & ID, const CommandContext & context, int flags);
   bool IsHidden(const PluginID & ID);

   /** Support for batch commands */
   bool SupportsAutomation(const PluginID & ID);
   wxString GetEffectParameters(const PluginID & ID);
   bool SetEffectParameters(const PluginID & ID, const wxString & params);
   bool PromptUser( const PluginID & ID, const EffectDialogFactory &factory,
      wxWindow &parent );
   bool HasPresets(const PluginID & ID);
   wxString GetPreset(const PluginID & ID, const wxString & params, wxWindow * parent);
   wxString GetDefaultPreset(const PluginID & ID);

private:
   void SetBatchProcessing(const PluginID & ID, bool start);
   struct UnsetBatchProcessing {
      PluginID mID;
      void operator () (EffectManager *p) const
         { if(p) p->SetBatchProcessing(mID, false); }
   };
   using BatchProcessingScope =
      std::unique_ptr< EffectManager, UnsetBatchProcessing >;
public:
   // RAII for the function above
   BatchProcessingScope SetBatchProcessing(const PluginID &ID)
   {
      SetBatchProcessing(ID, true); return BatchProcessingScope{ this, {ID} };
   }

   /** Allow effects to disable saving the state at run time */
   void SetSkipStateFlag(bool flag);
   bool GetSkipStateFlag();

   const PluginID & GetEffectByIdentifier(const CommandID & strTarget);

   /** Return an effect by its ID. */
   EffectUIHostInterface *GetEffect(const PluginID & ID);

private:
   AudacityCommand *GetAudacityCommand(const PluginID & ID);

   EffectMap mEffects;
   AudacityCommandMap mCommands;
   EffectOwnerMap mHostEffects;

   int mNumEffects;

   // Set true if we want to skip pushing state 
   // after processing at effect run time.
   bool mSkipStateFlag;
};

#endif
