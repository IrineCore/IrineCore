USE [dnmembership]
GO
/****** Object:  Table [dbo].[PetalIncome]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PetalIncome]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PetalIncome](
	[PetalIncomeID] [bigint] IDENTITY(1,1) NOT NULL,
	[AccountID] [int] NOT NULL,
	[PetalIncomeCode] [tinyint] NOT NULL,
	[PurchaseOrderID] [bigint] NULL,
	[AuditLogID] [int] NULL,
	[PetalAmount] [int] NOT NULL,
	[IncomeDate] [datetime2](0) NOT NULL,
	[PurchaseOrderDetailID] [bigint] NULL,
	[PetalIncomeFKey] [bigint] NULL,
	[ItemSerial] [bigint] SPARSE  NULL,
 CONSTRAINT [PK_PetalIncome] PRIMARY KEY CLUSTERED 
(
	[PetalIncomeID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_PetalIncome_IncomeDate]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PetalIncome]') AND name = N'IX_NN_PetalIncome_IncomeDate')
CREATE NONCLUSTERED INDEX [IX_NN_PetalIncome_IncomeDate] ON [dbo].[PetalIncome]
(
	[IncomeDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PetalIncome_PurchaseOrderID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PetalIncome]') AND name = N'IX_NN_PetalIncome_PurchaseOrderID')
CREATE NONCLUSTERED INDEX [IX_NN_PetalIncome_PurchaseOrderID] ON [dbo].[PetalIncome]
(
	[PurchaseOrderID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNF_PetalIncome_PetalIncomeFKey_PetalIncomeCode]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PetalIncome]') AND name = N'IX_NNF_PetalIncome_PetalIncomeFKey_PetalIncomeCode')
CREATE NONCLUSTERED INDEX [IX_NNF_PetalIncome_PetalIncomeFKey_PetalIncomeCode] ON [dbo].[PetalIncome]
(
	[PetalIncomeFKey] ASC,
	[PetalIncomeCode] ASC
)
WHERE ([PetalIncomeFKey] IS NOT NULL)
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_PetalIncome_AccountID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PetalIncome]') AND name = N'IX_NNI_PetalIncome_AccountID')
CREATE NONCLUSTERED INDEX [IX_NNI_PetalIncome_AccountID] ON [dbo].[PetalIncome]
(
	[AccountID] ASC
)
INCLUDE([PetalAmount]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_UNF_PetalIncome_ItemSerial]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PetalIncome]') AND name = N'IX_UNF_PetalIncome_ItemSerial')
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_PetalIncome_ItemSerial] ON [dbo].[PetalIncome]
(
	[ItemSerial] ASC
)
WHERE ([PetalIncomeCode]=(6) AND [ItemSerial] IS NOT NULL)
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_UNF_PetalIncome_PurchaseOrderDetailID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PetalIncome]') AND name = N'IX_UNF_PetalIncome_PurchaseOrderDetailID')
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_PetalIncome_PurchaseOrderDetailID] ON [dbo].[PetalIncome]
(
	[PurchaseOrderDetailID] ASC
)
WHERE ([PurchaseOrderDetailID] IS NOT NULL)
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_PetalIncome_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PetalIncome]'))
ALTER TABLE [dbo].[PetalIncome]  WITH CHECK ADD  CONSTRAINT [FK_Accounts_PetalIncome_AccountID] FOREIGN KEY([AccountID])
REFERENCES [dbo].[Accounts] ([AccountID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_PetalIncome_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PetalIncome]'))
ALTER TABLE [dbo].[PetalIncome] CHECK CONSTRAINT [FK_Accounts_PetalIncome_AccountID]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PurchaseOrderDetails_PetalIncome_PurchaseOrderDetailID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PetalIncome]'))
ALTER TABLE [dbo].[PetalIncome]  WITH CHECK ADD  CONSTRAINT [FK_PurchaseOrderDetails_PetalIncome_PurchaseOrderDetailID] FOREIGN KEY([PurchaseOrderDetailID])
REFERENCES [dbo].[PurchaseOrderDetails] ([PurchaseOrderDetailID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PurchaseOrderDetails_PetalIncome_PurchaseOrderDetailID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PetalIncome]'))
ALTER TABLE [dbo].[PetalIncome] CHECK CONSTRAINT [FK_PurchaseOrderDetails_PetalIncome_PurchaseOrderDetailID]
GO
