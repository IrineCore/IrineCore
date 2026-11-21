USE [dnmembership]
GO
/****** Object:  Table [dbo].[PurchaseOrderDetails]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PurchaseOrderDetails]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PurchaseOrderDetails](
	[PurchaseOrderDetailID] [bigint] IDENTITY(1,1) NOT NULL,
	[PurchaseOrderID] [bigint] NOT NULL,
	[ProductID] [int] NOT NULL,
	[ListPrice] [int] NOT NULL,
	[RefundFlag] [bit] NOT NULL,
	[RefundDate] [datetime2](0) SPARSE  NULL,
	[RefundableFlag] [bit] NOT NULL,
 CONSTRAINT [PK_PurchaseOrderDetails] PRIMARY KEY CLUSTERED 
(
	[PurchaseOrderDetailID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_PurchaseOrderDetails_ProductID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PurchaseOrderDetails]') AND name = N'IX_NN_PurchaseOrderDetails_ProductID')
CREATE NONCLUSTERED INDEX [IX_NN_PurchaseOrderDetails_ProductID] ON [dbo].[PurchaseOrderDetails]
(
	[ProductID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PurchaseOrderDetails_PurchaseOrderID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PurchaseOrderDetails]') AND name = N'IX_NN_PurchaseOrderDetails_PurchaseOrderID')
CREATE NONCLUSTERED INDEX [IX_NN_PurchaseOrderDetails_PurchaseOrderID] ON [dbo].[PurchaseOrderDetails]
(
	[PurchaseOrderID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_PurchaseOrderDetails_RefundFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[PurchaseOrderDetails] ADD  CONSTRAINT [DF_PurchaseOrderDetails_RefundFlag]  DEFAULT ((0)) FOR [RefundFlag]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_PurchaseOrderDetails_RefundableFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[PurchaseOrderDetails] ADD  CONSTRAINT [DF_PurchaseOrderDetails_RefundableFlag]  DEFAULT ((0)) FOR [RefundableFlag]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PurchaseOrders_PurchaseOrderDetails_PurchaseOrderID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PurchaseOrderDetails]'))
ALTER TABLE [dbo].[PurchaseOrderDetails]  WITH CHECK ADD  CONSTRAINT [FK_PurchaseOrders_PurchaseOrderDetails_PurchaseOrderID] FOREIGN KEY([PurchaseOrderID])
REFERENCES [dbo].[PurchaseOrders] ([PurchaseOrderID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PurchaseOrders_PurchaseOrderDetails_PurchaseOrderID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PurchaseOrderDetails]'))
ALTER TABLE [dbo].[PurchaseOrderDetails] CHECK CONSTRAINT [FK_PurchaseOrders_PurchaseOrderDetails_PurchaseOrderID]
GO
