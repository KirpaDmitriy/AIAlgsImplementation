import numpy as np
from sklearn.tree import DecisionTreeRegressor


class DimBoost:
    def __init__(self, n_iter, lr):
        self.models = []
        self.losses = []
        self.loss = lambda v1, v2: (v1 - v2) ** 2
        self.loss_grad = lambda v1, v2: (v1 - v2) * 2
        self.n_iter = n_iter
        self.lr = lr
        self.base_model = None
    
    def fit(self, X, y, model):
        self.base_model = model().fit(X, y)
        old_models_prediction = self.base_model.predict(X)
        y_l = np.array(y[y.columns[0]].to_list())

        for i in range(self.n_iter):
            grad = self.loss_grad(y_l, old_models_prediction)
            new_model = model().fit(X, (-1) * grad)
            
            new_model_corrections = self.lr * new_model.predict(X)
            old_models_prediction += new_model_corrections

            self.models.append(new_model)
            self.losses.append(
                self.loss(y_l, old_models_prediction)
                .mean()
            )

        return self
        
    def predict(self, X):
        base_prediction = self.base_model.predict(X)
        for model in self.models:
            base_prediction += self.lr * model.predict(X)
        return base_prediction



dimBoostModel = DimBoost(100, 0.01)
dimBoostModel.fit(train_X, train_y, DecisionTreeRegressor)
dimBoostModel.predict(test_X)