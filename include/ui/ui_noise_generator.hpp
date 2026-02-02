#pragma once

// Project includes
#include <common/_fmt.hpp>

// Other lib includes
#include <QWidget>

class UiNoiseGenerator : public QWidget {
  Q_OBJECT;
  using _base_ = QWidget;
  using _base_::_base_;

  public:
  UiNoiseGenerator( std::shared_ptr< spdlog::logger > logger, QWidget *parent = nullptr );
  ~UiNoiseGenerator();

  private:
  std::shared_ptr< spdlog::logger > logger_;

  private slots:
  // qt slots
  // void buttonClicked();
};
