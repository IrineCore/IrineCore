USE [dnmembership]
GO
/****** Object:  Table [dbo].[PurchaseOrders]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PurchaseOrders]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PurchaseOrders](
	[PurchaseOrderID] [bigint] IDENTITY(1351247,1) NOT NULL,
	[CharacterID] [bigint] NULL,
	[ReceiverCharacterID] [bigint] NULL,
	[GiftFlag] [bit] NOT NULL,
	[PCRoomFlag] [bit] NOT NULL,
	[Price] [int] NOT NULL,
	[IP] [int] NOT NULL,
	[OrderDate] [datetime2](0) NOT NULL,
	[OrderStatusCode] [tinyint] NOT NULL,
	[PayMethodCode] [tinyint] NULL,
	[OrderKey] [varchar](32) COLLATE Chinese_PRC_CI_AS NULL,
	[OrderResult] [int] NULL,
	[OrderRollbackResult] [int] NULL,
	[GiftReceiveFlag] [bit] NULL,
	[GiftMemo] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NULL,
	[GiftExpireDate] [smalldatetime] NULL,
	[GiftSenderName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS SPARSE  NULL,
	[CouponKey] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NULL,
	[GiftReceiveDate] [datetime2](0) SPARSE  NULL,
	[ContextKey] [varchar](32) COLLATE Chinese_PRC_CI_AS NULL,
	[GiftRejectFlag] [bit] NULL,
	[GiftRejectDate] [datetime2](0) SPARSE  NULL,
	[NewFlag] [bit] NULL,
	[PaidCashAmount] [int] NULL,
 CONSTRAINT [PK_PurchaseOrders] PRIMARY KEY CLUSTERED 
(
	[PurchaseOrderID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
SET ANSI_PADDING OFF
GO
/****** Object:  Index [IX_NN_PurchaseOrders_CharacterID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PurchaseOrders]') AND name = N'IX_NN_PurchaseOrders_CharacterID')
CREATE NONCLUSTERED INDEX [IX_NN_PurchaseOrders_CharacterID] ON [dbo].[PurchaseOrders]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PurchaseOrders_OrderDate]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PurchaseOrders]') AND name = N'IX_NN_PurchaseOrders_OrderDate')
CREATE NONCLUSTERED INDEX [IX_NN_PurchaseOrders_OrderDate] ON [dbo].[PurchaseOrders]
(
	[OrderDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_PurchaseOrders_ReceiverCharacterID_GiftFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PurchaseOrders]') AND name = N'IX_NNI_PurchaseOrders_ReceiverCharacterID_GiftFlag')
CREATE NONCLUSTERED INDEX [IX_NNI_PurchaseOrders_ReceiverCharacterID_GiftFlag] ON [dbo].[PurchaseOrders]
(
	[ReceiverCharacterID] ASC,
	[GiftFlag] ASC
)
INCLUDE([GiftRejectFlag]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_PurchaseOrders_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PurchaseOrders]'))
ALTER TABLE [dbo].[PurchaseOrders]  WITH CHECK ADD  CONSTRAINT [FK_Characters_PurchaseOrders_CharacterID] FOREIGN KEY([CharacterID])
REFERENCES [dbo].[Characters] ([CharacterID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_PurchaseOrders_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PurchaseOrders]'))
ALTER TABLE [dbo].[PurchaseOrders] CHECK CONSTRAINT [FK_Characters_PurchaseOrders_CharacterID]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_PurchaseOrders_ReceiverCharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PurchaseOrders]'))
ALTER TABLE [dbo].[PurchaseOrders]  WITH CHECK ADD  CONSTRAINT [FK_Characters_PurchaseOrders_ReceiverCharacterID] FOREIGN KEY([ReceiverCharacterID])
REFERENCES [dbo].[Characters] ([CharacterID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_PurchaseOrders_ReceiverCharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[PurchaseOrders]'))
ALTER TABLE [dbo].[PurchaseOrders] CHECK CONSTRAINT [FK_Characters_PurchaseOrders_ReceiverCharacterID]
GO
