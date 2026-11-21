USE [dnmembership]
GO
/****** Object:  Table [dbo].[CashOutgo]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CashOutgo]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CashOutgo](
	[CashOutgoID] [int] IDENTITY(1,1) NOT NULL,
	[AccountID] [int] NOT NULL,
	[CashOutgoCode] [tinyint] NOT NULL,
	[PurchaseOrderID] [bigint] NULL,
	[CashAmount] [int] NOT NULL,
	[OutgoDate] [datetime2](0) NOT NULL
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NC_CashOutgo_OutgoDate]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CashOutgo]') AND name = N'IX_NC_CashOutgo_OutgoDate')
CREATE CLUSTERED INDEX [IX_NC_CashOutgo_OutgoDate] ON [dbo].[CashOutgo]
(
	[OutgoDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_CashOutgo]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CashOutgo]') AND name = N'PK_CashOutgo')
ALTER TABLE [dbo].[CashOutgo] ADD  CONSTRAINT [PK_CashOutgo] PRIMARY KEY NONCLUSTERED 
(
	[CashOutgoID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_CashOutgo_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CashOutgo]'))
ALTER TABLE [dbo].[CashOutgo]  WITH CHECK ADD  CONSTRAINT [FK_Accounts_CashOutgo_AccountID] FOREIGN KEY([AccountID])
REFERENCES [dbo].[Accounts] ([AccountID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_CashOutgo_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CashOutgo]'))
ALTER TABLE [dbo].[CashOutgo] CHECK CONSTRAINT [FK_Accounts_CashOutgo_AccountID]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PurchaseOrders_CashOutgo_PurchaseOrderID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CashOutgo]'))
ALTER TABLE [dbo].[CashOutgo]  WITH CHECK ADD  CONSTRAINT [FK_PurchaseOrders_CashOutgo_PurchaseOrderID] FOREIGN KEY([PurchaseOrderID])
REFERENCES [dbo].[PurchaseOrders] ([PurchaseOrderID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PurchaseOrders_CashOutgo_PurchaseOrderID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CashOutgo]'))
ALTER TABLE [dbo].[CashOutgo] CHECK CONSTRAINT [FK_PurchaseOrders_CashOutgo_PurchaseOrderID]
GO
