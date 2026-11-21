USE [dnmembership]
GO
/****** Object:  Table [dbo].[PetalOutgo]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PetalOutgo]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PetalOutgo](
	[PetalOutgoID] [int] IDENTITY(1,1) NOT NULL,
	[PetalIncomeID] [bigint] NOT NULL,
	[PetalOutgoCode] [tinyint] NOT NULL,
	[PurchaseOrderID] [bigint] NULL,
	[PetalAmount] [int] NOT NULL,
	[OutgoDate] [datetime2](0) NOT NULL,
	[PetalOutgoFKey] [bigint] NULL,
 CONSTRAINT [PK_PetalOutgo] PRIMARY KEY CLUSTERED 
(
	[PetalOutgoID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_PetalOutgo_PetalIncomeID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PetalOutgo]') AND name = N'IX_NN_PetalOutgo_PetalIncomeID')
CREATE NONCLUSTERED INDEX [IX_NN_PetalOutgo_PetalIncomeID] ON [dbo].[PetalOutgo]
(
	[PetalIncomeID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PetalOutgo_PurchaseOrderID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PetalOutgo]') AND name = N'IX_NN_PetalOutgo_PurchaseOrderID')
CREATE NONCLUSTERED INDEX [IX_NN_PetalOutgo_PurchaseOrderID] ON [dbo].[PetalOutgo]
(
	[PurchaseOrderID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNF_PetalOutgo_PetalOutgoFKey_PetalOutgoCode]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PetalOutgo]') AND name = N'IX_NNF_PetalOutgo_PetalOutgoFKey_PetalOutgoCode')
CREATE NONCLUSTERED INDEX [IX_NNF_PetalOutgo_PetalOutgoFKey_PetalOutgoCode] ON [dbo].[PetalOutgo]
(
	[PetalOutgoFKey] ASC,
	[PetalOutgoCode] ASC
)
WHERE ([PetalOutgoFKey] IS NOT NULL)
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_PetalOutgo_OutgoDate]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PetalOutgo]') AND name = N'IX_NNI_PetalOutgo_OutgoDate')
CREATE NONCLUSTERED INDEX [IX_NNI_PetalOutgo_OutgoDate] ON [dbo].[PetalOutgo]
(
	[OutgoDate] ASC
)
INCLUDE([PetalIncomeID]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PetalIncome_PetalOutgo_PetalIncomeID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PetalOutgo]'))
ALTER TABLE [dbo].[PetalOutgo]  WITH CHECK ADD  CONSTRAINT [FK_PetalIncome_PetalOutgo_PetalIncomeID] FOREIGN KEY([PetalIncomeID])
REFERENCES [dbo].[PetalIncome] ([PetalIncomeID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PetalIncome_PetalOutgo_PetalIncomeID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PetalOutgo]'))
ALTER TABLE [dbo].[PetalOutgo] CHECK CONSTRAINT [FK_PetalIncome_PetalOutgo_PetalIncomeID]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PurchaseOrders_PetalOutgo_PurchaseOrderID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PetalOutgo]'))
ALTER TABLE [dbo].[PetalOutgo]  WITH CHECK ADD  CONSTRAINT [FK_PurchaseOrders_PetalOutgo_PurchaseOrderID] FOREIGN KEY([PurchaseOrderID])
REFERENCES [dbo].[PurchaseOrders] ([PurchaseOrderID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PurchaseOrders_PetalOutgo_PurchaseOrderID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PetalOutgo]'))
ALTER TABLE [dbo].[PetalOutgo] CHECK CONSTRAINT [FK_PurchaseOrders_PetalOutgo_PurchaseOrderID]
GO
