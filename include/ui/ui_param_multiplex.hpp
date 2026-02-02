#pragma once

// Project includes
#include <common/_fmt.hpp>

// Other lib includes
#include <QWidget>

class UiParamMultiplex : public QWidget {
  Q_OBJECT;
  using _base_ = QWidget;
  using _base_::_base_;

  public:
  UiParamMultiplex( std::shared_ptr< spdlog::logger > logger, QWidget *parent = nullptr );
  ~UiParamMultiplex();

  private:
  std::shared_ptr< spdlog::logger > logger_;

  private slots:
  // qt slots
  // void buttonClicked();
};
