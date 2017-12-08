#include "primitives_display.h"

namespace planning_rviz_plugins {
  PrimitivesDisplay::PrimitivesDisplay() {
    num_property_ =
      new rviz::IntProperty("Num of samples", 4, "Number of samples to display.",
          this, SLOT(updateNum()));
    pos_color_property_ =
      new rviz::ColorProperty("PosColor", QColor(204, 51, 204), "Color to draw the Pos.",
          this, SLOT(updatePosColorAndAlpha()));
    vel_color_property_ =
      new rviz::ColorProperty("VelColor", QColor(85, 85, 255), "Color to draw the Vel.",
          this, SLOT(updateVelColorAndAlpha()));
    acc_color_property_ =
      new rviz::ColorProperty("AccColor", QColor(10, 200, 55), "Color to draw the Acc.",
          this, SLOT(updateAccColorAndAlpha()));
    jrk_color_property_ =
      new rviz::ColorProperty("JrkColor", QColor(200, 20, 55), "Color to draw the Acc.",
          this, SLOT(updateJrkColorAndAlpha()));
    pos_scale_property_ =
      new rviz::FloatProperty("PosScale", 0.02, "0.02 is the default value.",
          this, SLOT(updatePosScale()));
    vel_scale_property_ =
      new rviz::FloatProperty("VelScale", 0.02, "0.02 is the default value.",
          this, SLOT(updateVelScale()));
    acc_scale_property_ =
      new rviz::FloatProperty("AccScale", 0.02, "0.02 is the default value.",
          this, SLOT(updateAccScale()));
    jrk_scale_property_ =
      new rviz::FloatProperty("JrkScale", 0.02, "0.02 is the default value.",
          this, SLOT(updateJrkScale()));
    vel_vis_property_ =
      new rviz::BoolProperty("Visualize Vel", 0, "Visualize Vel?",
          this, SLOT(updateVelVis()));
    acc_vis_property_ =
      new rviz::BoolProperty("Visualize Acc", 0, "Visualize Acc?",
          this, SLOT(updateAccVis()));
    jrk_vis_property_ =
      new rviz::BoolProperty("Visualize Jrk", 0, "Visualize Jrk?",
          this, SLOT(updateJrkVis()));
    history_length_property_ =
      new rviz::IntProperty("History Length", 1, "Number of msg to display.",
          this, SLOT(updateHistoryLength()));
    history_length_property_->setMin(1);
    history_length_property_->setMax(10000);
 }

  void PrimitivesDisplay::onInitialize() {
    MFDClass::onInitialize();
    updateHistoryLength();
  }

  PrimitivesDisplay::~PrimitivesDisplay() {}

  void PrimitivesDisplay::reset() {
    MFDClass::reset();
    visuals_.clear();
  }

  void PrimitivesDisplay::updateVelVis() {
    bool vis = vel_vis_property_->getBool();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setVelVis(vis);
    visualizeMessage();
  }

  void PrimitivesDisplay::updateAccVis() {
    bool vis = acc_vis_property_->getBool();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setAccVis(vis);
    visualizeMessage();
  }

  void PrimitivesDisplay::updateJrkVis() {
    bool vis = jrk_vis_property_->getBool();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setJrkVis(vis);
    visualizeMessage();
  }

  void PrimitivesDisplay::updatePosColorAndAlpha() {
    float alpha = 1;
    Ogre::ColourValue color = pos_color_property_->getOgreColor();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setPosColor(color.r, color.g, color.b, alpha);
  }

  void PrimitivesDisplay::updateVelColorAndAlpha() {
    float alpha = 1;
    Ogre::ColourValue color = vel_color_property_->getOgreColor();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setVelColor(color.r, color.g, color.b, alpha);
  }

  void PrimitivesDisplay::updateAccColorAndAlpha() {
    float alpha = 1;
    Ogre::ColourValue color = acc_color_property_->getOgreColor();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setAccColor(color.r, color.g, color.b, alpha);
  }

  void PrimitivesDisplay::updateJrkColorAndAlpha() {
    float alpha = 1;
    Ogre::ColourValue color = jrk_color_property_->getOgreColor();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setJrkColor(color.r, color.g, color.b, alpha);
  }

  void PrimitivesDisplay::updatePosScale() {
    float s = pos_scale_property_->getFloat();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setPosScale(s);
  }

  void PrimitivesDisplay::updateVelScale() {
    float s = vel_scale_property_->getFloat();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setVelScale(s);
  }

  void PrimitivesDisplay::updateAccScale() {
    float s = acc_scale_property_->getFloat();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setAccScale(s);
  }

  void PrimitivesDisplay::updateJrkScale() {
    float s = jrk_scale_property_->getFloat();
    for (size_t i = 0; i < visuals_.size(); i++)
      visuals_[i]->setJrkScale(s);
  }

  void PrimitivesDisplay::updateHistoryLength() {
    visuals_.rset_capacity(history_length_property_->getInt());
  }

  void PrimitivesDisplay::processMessage(const planning_ros_msgs::Primitives::ConstPtr &msg) {
    if (!context_->getFrameManager()->getTransform(
          msg->header.frame_id, msg->header.stamp, position_, orientation_)) {
      ROS_DEBUG("Error transforming from frame '%s' to frame '%s'",
          msg->header.frame_id.c_str(), qPrintable(fixed_frame_));
      return;
    }

    prs_msg_ = *msg;

    visualizeMessage();
  }

  void PrimitivesDisplay::updateNum() {
    visualizeMessage();
  }

  void PrimitivesDisplay::visualizeMessage() {
    if (prs_msg_.primitives.empty())
      return;

    boost::shared_ptr<PrimitiveVisual> visual;
    if (visuals_.full())
      visual = visuals_.front();
    else
      visual.reset(new PrimitiveVisual(context_->getSceneManager(), scene_node_));

    float n = num_property_->getInt();
    visual->setNum(n);

    bool vel_vis = vel_vis_property_->getBool();
    visual->setVelVis(vel_vis);

    bool acc_vis = acc_vis_property_->getBool();
    visual->setAccVis(acc_vis);

    bool jrk_vis = jrk_vis_property_->getBool();
    visual->setJrkVis(jrk_vis);

    visual->setMessage(prs_msg_.primitives.front());
    for(int i = 1; i < (int) prs_msg_.primitives.size(); i++)
      visual->addMessage(prs_msg_.primitives[i]);

    visual->setFramePosition(position_);
    visual->setFrameOrientation(orientation_);

    float pos_scale = pos_scale_property_->getFloat();
    visual->setPosScale(pos_scale);
    float vel_scale = vel_scale_property_->getFloat();
    visual->setVelScale(vel_scale);
    float acc_scale = acc_scale_property_->getFloat();
    visual->setAccScale(acc_scale);
    float jrk_scale = jrk_scale_property_->getFloat();
    visual->setJrkScale(jrk_scale);

    Ogre::ColourValue pos_color = pos_color_property_->getOgreColor();
    visual->setPosColor(pos_color.r, pos_color.g, pos_color.b, 1);
    Ogre::ColourValue vel_color = vel_color_property_->getOgreColor();
    visual->setVelColor(vel_color.r, vel_color.g, vel_color.b, 1);
    Ogre::ColourValue acc_color = acc_color_property_->getOgreColor();
    visual->setAccColor(acc_color.r, acc_color.g, acc_color.b, 1);
    Ogre::ColourValue jrk_color = jrk_color_property_->getOgreColor();
    visual->setJrkColor(jrk_color.r, jrk_color.g, jrk_color.b, 1);

    visuals_.push_back(visual);
  }

}

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(planning_rviz_plugins::PrimitivesDisplay, rviz::Display)
