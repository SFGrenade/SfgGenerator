#pragma once

// Project includes
#include "ui/SfgSlider.hpp"

// Project includes
#include <common/_fmt.hpp>

// Other lib includes
#include <QGridLayout>
#include <QLabel>
#include <QWidget>

class UiAudioLerpEffect : public QWidget {
  Q_OBJECT;
  using _base_ = QWidget;
  using _base_::_base_;

  public:
  UiAudioLerpEffect( std::shared_ptr< spdlog::logger > logger, QWidget *parent = nullptr );
  virtual ~UiAudioLerpEffect();

  private:
  std::shared_ptr< spdlog::logger > logger_;
  QGridLayout* layout_ = nullptr;
  QLabel* mainLabel_ = nullptr;
  QLabel* leftLabel_ = nullptr;
  SfgSlider* abSlider_ = nullptr;
  QLabel* rightLabel_ = nullptr;

  private:
  int const abSliderMinValue_ = 0;
  int const abSliderMaxValue_ = 1 << 30;

  private slots:
  // qt slots
  // void buttonClicked();
};
