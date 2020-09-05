#ifndef PROJECTFILE_H
#define PROJECTFILE_H

#include <mutex>
#include <condition_variable>

#include "src/filetypes/hkxfile.h"
#include "src/animData/skyrimanimdata.h"
#include "src/animSetData/skyrimanimsetdata.h"

class CharacterFile;
class BehaviorFile;

namespace UI {
	class hkbBehaviorReferenceGenerator;
	class hkbStateMachine;
	class hkaSkeleton;
	class hkbFootIkDriverInfo;
	class hkbHandIkDriverInfo;
	class ProjectUI;
}

class ProjectFile final: public HkxFile
{
    friend class BehaviorFile;
    friend class MainWindow;
    friend class UI::ProjectUI;
public:
    ProjectFile(MainWindow *window, const QString & name, bool autogeneratedata = false, const QString & relativecharacterfilepath = "");
    ProjectFile& operator=(const ProjectFile&) = delete;
    ProjectFile(const ProjectFile &) = delete;
    ~ProjectFile();
public:
    bool doesBehaviorExist(const QString &behaviorname) const;
    QString getCharacterFilePathAt(int index) const;
    bool isClipGenNameTaken(const QString & name) const;
    bool readAnimationData(const QString &filename, const QStringList &behaviorfilenames = QStringList());
    bool readAnimationSetData(const QString & filename);
    int getAnimationIndex(const QString & name) const;
    bool isAnimationUsed(const QString & animationname) const;
    QStringList getAnimationNames() const;
    QString findAnimationNameFromEncryptedData(const QString & encryptedname) const;
    bool isProjectNameTaken() const;
    QString getProjectName() const;
    qreal getAnimationDurationFromAnimData(const QString & animationname) const;
    bool appendAnimation(SkyrimAnimationMotionData *motiondata);
    SkyrimAnimationMotionData getAnimationMotionData(int animationindex) const;
    bool isNameUniqueInProject(UI::HkxObject *object, const QString & filenametoignore) const;
    bool hasAnimData() const;
    bool hasAnimSetData() const;
    void writeOrderOfFiles() const;
	UI::hkbStateMachine * findRootStateMachineFromBehavior(const QString behaviorname) const;
    QString getBehaviorDirectoryName() const;
    QString getProjectAnimationsPath() const;
	UI::HkxSharedPtr * findProjectStringData(long ref);
    AnimCacheProjectData *getProjectCacheData() const;
	UI::HkxSharedPtr * findProjectData(long ref);
protected:
    bool parse();
	virtual bool parseBinary();
    bool link();
private:
    void write();
    bool addObjectToFile(UI::HkxObject *obj, long ref = -1);
    void setCharacterFile(CharacterFile *file);
    void addHandIK();
    void addFootIK();
    void disableHandIK();
    void disableFootIK();
	UI::hkbHandIkDriverInfo * getHandIkDriverInfo() const;
	UI::hkbFootIkDriverInfo * getFootIkDriverInfo() const;
    UI::hkaSkeleton * getSkeleton(bool isragdoll = false) const;
    ProjectAnimData * getAnimDataAt(const QString &projectname) const;
	UI::HkxObject * getCharacterStringData() const;
	UI::HkxObject * getCharacterData() const;
    QString detectErrorsInProject();
    QString detectErrorsInBehavior(const QString & filename);
    void removeUnreferencedFiles(const UI::hkbBehaviorReferenceGenerator *gentoignore);
    void ensureAllRefedAnimationsExist();
    bool merge(ProjectFile *recessiveproject, bool isFNIS = false);
    bool mergeAnimationCaches(ProjectFile *recessiveproject);
    void addProjectToAnimData();
    bool removeClipGenFromAnimData(const QString & animationname, const QString &clipname, const QString &variablename = "");
    bool removeAnimationFromAnimData(const QString & name);
    bool appendClipGeneratorAnimData(const QString & name);
    void setLocalTimeForClipGenAnimData(const QString &clipname, int triggerindex, qreal time);
    void setEventNameForClipGenAnimData(const QString &clipname, int triggerindex, const QString &eventname);
    void setClipNameAnimData(const QString &oldclipname, const QString &newclipname);
    void setAnimationIndexForClipGen(int index, const QString &clipGenName);
    void setPlaybackSpeedAnimData(const QString & clipGenName, qreal speed);
    void setCropStartAmountLocalTimeAnimData(const QString & clipGenName, qreal time);
    void setCropEndAmountLocalTimeAnimData(const QString & clipGenName, qreal time);
    void appendClipTriggerToAnimData(const QString & clipGenName, const QString & eventname);
    void removeClipTriggerToAnimDataAt(const QString & clipGenName, int index);
    void setAnimationIndexDuration(int indexofanimationlist, int animationindex, qreal duration);
    void generateAnimClipDataForProject();
    void loadEncryptedAnimationNames();
    void addEncryptedAnimationName(const QString & unencryptedname);
    void removeEncryptedAnimationName(int index);
    void deleteBehaviorFile(const QString & filename);
private:
    CharacterFile *character;
    QVector <BehaviorFile *> behaviorFiles;
    UI::HkxSharedPtr stringData;
	UI::HkxSharedPtr projectData;
    long largestRef;
    SkyrimAnimData *skyrimAnimData;
    SkyrimAnimSetData *skyrimAnimSetData;
    int projectIndex;
    QString projectName;
    QString projectFolderName;
    QString projectAnimationsPath;
    QStringList encryptedAnimationNames;
    mutable std::mutex mutex;
    std::condition_variable conditionVar;
};

#endif // PROJECTFILE_H
