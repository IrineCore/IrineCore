USE [dnmembership]
GO
/****** Object:  Table [dbo].[SelectedItems]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SelectedItems]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[SelectedItems](
	[SelectedItemID] [bigint] IDENTITY(1,1) NOT NULL,
	[PurchaseOrderDetailID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[GiveFailFlag] [bit] NOT NULL,
	[ItemOption] [tinyint] NULL,
 CONSTRAINT [PK_SelectedItems] PRIMARY KEY CLUSTERED 
(
	[SelectedItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NNI_SelectedItems_PurchaseOrderDetailID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[SelectedItems]') AND name = N'IX_NNI_SelectedItems_PurchaseOrderDetailID')
CREATE NONCLUSTERED INDEX [IX_NNI_SelectedItems_PurchaseOrderDetailID] ON [dbo].[SelectedItems]
(
	[PurchaseOrderDetailID] ASC
)
INCLUDE([ItemID],[GiveFailFlag]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_SelectedItems_GiveFailFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[SelectedItems] ADD  CONSTRAINT [DF_SelectedItems_GiveFailFlag]  DEFAULT ((0)) FOR [GiveFailFlag]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PurchaseOrderDetails_SelectedItems_PurchaseOrderDetailID]') AND parent_object_id = OBJECT_ID(N'[dbo].[SelectedItems]'))
ALTER TABLE [dbo].[SelectedItems]  WITH CHECK ADD  CONSTRAINT [FK_PurchaseOrderDetails_SelectedItems_PurchaseOrderDetailID] FOREIGN KEY([PurchaseOrderDetailID])
REFERENCES [dbo].[PurchaseOrderDetails] ([PurchaseOrderDetailID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PurchaseOrderDetails_SelectedItems_PurchaseOrderDetailID]') AND parent_object_id = OBJECT_ID(N'[dbo].[SelectedItems]'))
ALTER TABLE [dbo].[SelectedItems] CHECK CONSTRAINT [FK_PurchaseOrderDetails_SelectedItems_PurchaseOrderDetailID]
GO
