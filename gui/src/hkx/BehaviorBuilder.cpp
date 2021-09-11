#include "BehaviorBuilder.h"

#include <src/hkx/HkxItemEvent.h>
#include <src/hkx/HkxItemVar.h>

#include <hkbBehaviorGraph_1.h>
#include <hkbStateMachineTransitionInfo_1.h>
#include <hkbStateMachineTimeInterval_0.h>
#include <hkbVariableBindingSet_2.h>
#include <hkbBehaviorReferenceGenerator_0.h>

using namespace ckcmd::HKX;

std::vector<const hkClass*> BehaviorBuilder::getHandledClasses()
{
	return {
		&hkbBehaviorGraphClass,
		&hkbBehaviorGraphDataClass,
		&hkbBehaviorGraphStringDataClass,
		&hkbBehaviorReferenceGeneratorClass,
		&hkbClipGeneratorClass
	};
};

std::vector<member_id_t> BehaviorBuilder::getEventFields() {
	return {
		{&hkbStateMachineTransitionInfoClass, hkbStateMachineTransitionInfoClass.getMemberByName("eventId")},
		{&hkbStateMachineTimeIntervalClass, hkbStateMachineTimeIntervalClass.getMemberByName("enterEventId")},
		{&hkbStateMachineTimeIntervalClass, hkbStateMachineTimeIntervalClass.getMemberByName("exitEventId")}
	};
}

std::vector<member_id_t> BehaviorBuilder::getVariableFields()
{
	return {
		{ &hkbVariableBindingSetBindingClass, hkbVariableBindingSetBindingClass.getMemberByName("variableIndex") }
	};
}

std::vector<member_id_t> BehaviorBuilder::getHandledFields() {
	std::vector<member_id_t> result;
	auto events = getEventFields();
	auto variables = getVariableFields();
	result.reserve(events.size() + variables.size());
	result.insert(result.end(), events.begin(), events.end());
	result.insert(result.end(), variables.begin(), variables.end());
	if (_skeleton_builder != NULL) {
		auto skeleton = _skeleton_builder->getHandledFields();
		result.reserve(events.size() + variables.size() + skeleton.size());
		result.insert(result.end(), skeleton.begin(), skeleton.end());
	}
	return result;
}

BehaviorBuilder::BehaviorBuilder(ResourceManager& manager, CacheEntry* cache, size_t file_index, ProjectNode* animationsNode) :
	_manager(manager),
	_cache(cache),
	_file_index(file_index),
	_animationsNode(animationsNode)
{
}

void BehaviorBuilder::buildEvents(const buildContext& context)
{
	if (context.data->m_eventInfos.getSize() > 0)
	{
		auto events_node = context.parent->appendChild(
			_manager.createSupport(
				_file_index,
				{
					"Events"
				},
				context.parent)
		);

		for (int i = 0; i < context.data->m_eventInfos.getSize(); i++)
		{
			auto event_name = context.string_data->m_eventNames[i];
			auto event_info = context.data->m_eventInfos[i];

			QString name = QString("[%1] %2").arg(i).arg(event_name.cString());
			auto event_node = events_node->appendChild(
				_manager.createEventNode(
					_file_index,
					{
						name,
						(unsigned long long)_manager.at(_file_index, context.object_index),
						(int)_file_index,
						i,
					},
					events_node)
			);
		}
	}
}
void BehaviorBuilder::buildVariables(const buildContext& context)
{
	if (context.data->m_variableInfos.getSize() > 0)
	{
		auto variables_node = context.parent->appendChild(
			_manager.createSupport(
				_file_index,
				{
					"Variables"
				},
				context.parent)
		);

		for (int i = 0; i < context.data->m_variableInfos.getSize(); i++)
		{
			auto variable_name = context.string_data->m_variableNames[i];
			auto variable_info = context.data->m_variableInfos[i];

			QString name = QString("[%1] %2").arg(i).arg(variable_name.cString());
			auto variable_node = variables_node->appendChild(
				_manager.createEventNode(
					_file_index,
					{
						name,
						(unsigned long long)_manager.at(_file_index, context.object_index),
						(int)_file_index,
						i
					},
					variables_node)
			);
		}
	}
}
void BehaviorBuilder::buildProperties(const buildContext& context)
{
	if (context.data->m_variableInfos.getSize() > 0)
	{
		auto properties_node = context.parent->appendChild(
			_manager.createSupport(
				_file_index,
				{
					"Properties"
				},
				context.parent)
		);

		for (int i = 0; i < context.data->m_characterPropertyInfos.getSize(); i++)
		{
			auto property_name = context.string_data->m_characterPropertyNames[i];
			auto property_info = context.data->m_characterPropertyInfos[i];

			QString name = QString("[%1] %2").arg(i).arg(property_name.cString());
			auto variable_node = properties_node->appendChild(
				_manager.createPropertyNode(
					_file_index,
					{
						name,
						(unsigned long long)_manager.at(_file_index, context.object_index),
						_file_index,
						i
					},
					properties_node)
			);
		}
	}
}


//TODO: common to all builders
ProjectNode* BehaviorBuilder::buildBranch(hkVariant& variant, ProjectNode* root_node, const fs::path& path) {
	QString display_name = variant.m_class->getName();
	//check if the object has a name we can display
	auto member = variant.m_class->getMemberByName("name");
	if (HK_NULL != member)
	{
		auto member_ptr = ((char*)variant.m_object) + member->getOffset();
		auto c_str_ptr = (char*)*(uintptr_t*)(member_ptr);
		display_name = QString("%1 \"%2\"").arg(display_name).arg(c_str_ptr);
	}

	auto object_index = _manager.findIndex(path, variant.m_object);
	QString name = QString("[%1] %2").arg(object_index).arg(display_name);
	return root_node->appendChild(
		_manager.createHkxNode(
			_file_index,
			{
				name,
				(unsigned long long)_manager.at(_file_index, object_index),
				(unsigned long long) root_node->isVariant() ? root_node->data(1) : 0,
				(int)_file_index
			},
			root_node));
}

void BehaviorBuilder::addCacheToClipNode(ProjectNode* clip_node, const hkbClipGenerator* clip)
{
	auto events = _cache->getEvents(clip->m_name.cString());
	const std::string delimiter = ":";

	for (const auto& clip_event : events) {
		auto delim_pos = clip_event.find(delimiter);
		std::string event_name = clip_event.substr(0, delim_pos);
		float time = stof(clip_event.substr(++delim_pos, clip_event.size() - 1));
		//sanity check;
		size_t event_index = (size_t )-1;
		for (int i = 0; i < _strings->m_eventNames.getSize(); i++) {
			if (_strings->m_eventNames[i].cString() == event_name)
			{
				event_index = i;
				break;
			}
		}
		if (event_index == (size_t)-1)
		{
			LOG << "Event not found into behavior: " << event_name << log_endl;
		}

		clip_node->appendChild(
			_manager.createClipEventNode(
				_file_index,
				{
					QString::fromStdString(clip_event),
					time,
					event_index
				},
				clip_node)
		);
	}

	auto movements = _cache->findMovement(clip->m_name.cString());
	if (movements.empty())
	{
		LOG << "Unable to find the movement for " << clip->m_name.cString() << log_endl;
	}
	if (!movements.empty() && _animationsNode != nullptr) {
		for (int i = 0; i < _animationsNode->childCount(); i++)
		{
			if (_animationsNode->child(i)->data(0).value<QString>() == clip->m_animationName.cString()) {
				_animationsNode->child(i)->appendData(QString::fromStdString(movements));
				break;
			}
		}
	}
}

ProjectNode* BehaviorBuilder::visit(
	const fs::path& _file,
	int object_index,
	ProjectNode* parent)
{
	auto* variant = _manager.at(_file, object_index);
	if (variant->m_class == &hkbBehaviorGraphClass)
	{
		hkbBehaviorGraph* graph = (hkbBehaviorGraph*)variant->m_object;
		auto data = graph->m_data;
		_strings = graph->m_data->m_stringData;

		buildContext context =
		{
			data,
			_strings,
			_file,
			object_index,
			parent,
		};

		buildEvents(context);
		buildVariables(context);
		buildProperties(context);
	}
	else if (variant->m_class == &hkbBehaviorReferenceGeneratorClass) {
		hkbBehaviorReferenceGenerator* reference = (hkbBehaviorReferenceGenerator*)variant->m_object;
		_referenced_behaviors.insert(reference->m_behaviorName.cString());
		return buildBranch(*variant, parent, _file);
	}
	else if (variant->m_class == &hkbClipGeneratorClass) {
		auto clip_node = buildBranch(*variant, parent, _file);
		hkbClipGenerator* clip = (hkbClipGenerator*)variant->m_object;
		if (_cache->hasCache()) {
			addCacheToClipNode(clip_node, clip);
		}
		return clip_node;
	}
	return parent;
}

QVariant BehaviorBuilder::handle(void* value, const hkClass* hkclass, const hkClassMember* hkmember, const hkVariant* container, const hkVariant* parent_container)
{
	auto events = getEventFields();
	if (std::find_if(events.begin(), events.end(), 
		[&hkclass, &hkmember](const member_id_t& element){ return element.first == hkclass && element.second == hkmember; }) != events.end())
	{
		return HkxItemEvent(this, *(int*)value);
	}
	auto variables = getVariableFields();
	if (std::find_if(variables.begin(), variables.end(),
		[&hkclass, &hkmember](const member_id_t& element) { return element.first == hkclass && element.second == hkmember; }) != variables.end())
	{
		return HkxItemVar(this, *(int*)value);
	}
	auto bones = _skeleton_builder->getHandledFields();
	if (std::find_if(bones.begin(), bones.end(),
		[&hkclass, &hkmember](const member_id_t& element) { return element.first == hkclass && element.second == hkmember; }) != bones.end())
	{
		return _skeleton_builder->handle(value, hkclass, hkmember, container, parent_container);
	}
	return "BehaviorBuilder - Not set";
}

QStringList BehaviorBuilder::getEvents() const
{
	QStringList out;
	for (int i = 0; i < _strings->m_eventNames.getSize(); i++)
	{
		out << _strings->m_eventNames[i].cString();
	}
	return out;
}

QString BehaviorBuilder::getEvent(size_t index) const
{
	if (index < _strings->m_eventNames.getSize())
		return _strings->m_eventNames[index].cString();
	return "No Event";
}

QStringList BehaviorBuilder::getVariables() const
{
	QStringList out;
	for (int i = 0; i < _strings->m_variableNames.getSize(); i++)
	{
		out << _strings->m_variableNames[i].cString();
	}
	return out;
}

QString BehaviorBuilder::getVariable(size_t index) const
{
	if (index < _strings->m_variableNames.getSize())
		return _strings->m_variableNames[index].cString();
	return "No Variable";
}