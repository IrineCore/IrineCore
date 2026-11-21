USE [dnstaging]
GO
/****** Object:  Table [dbo].[PurchaseOrders]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[PurchaseOrders](
	[PurchaseOrderID] [bigint] NOT NULL,
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
	[ContextKey] [varchar](32) COLLATE Chinese_PRC_CI_AS NULL,
	[CouponKey] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NULL,
	[PaidCashAmount] [int] NULL,
 CONSTRAINT [PK_PurchaseOrders] PRIMARY KEY CLUSTERED 
(
	[PurchaseOrderID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
/****** Object:  Index [IX_NN_PurchaseOrders_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PurchaseOrders_CharacterID] ON [dbo].[PurchaseOrders]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PurchaseOrders_OrderDate]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PurchaseOrders_OrderDate] ON [dbo].[PurchaseOrders]
(
	[OrderDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PurchaseOrders_ReceiverCharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PurchaseOrders_ReceiverCharacterID] ON [dbo].[PurchaseOrders]
(
	[ReceiverCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
