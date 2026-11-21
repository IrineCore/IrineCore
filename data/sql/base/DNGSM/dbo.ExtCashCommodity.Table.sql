USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtCashCommodity]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtCashCommodity](
	[id] [int] NULL,
	[_SN] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Category] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SubCategory] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NameID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_DescriptionID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ItemID01] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ItemID02] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ItemID03] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ItemID04] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ItemID05] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_LinkSN01] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_LinkSN02] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_LinkSN03] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_LinkSN04] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_LinkSN05] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Period] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_VIPSell] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_VIPLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Pay] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_PaySale] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Price] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Count] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Priority] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_OnSale] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Limit] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ReserveGive] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Reserve] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_validity] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ReserveAble] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_PresentAble] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ItemSort] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_PriceFix] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CartAble] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_KeyLinkSN] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_OnSaleREAL] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_VIPPoint] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SNJPN] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SyncDate] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_OverlapBuy] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NoRefund] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CreditAble] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CreditAbleLV] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_UseCoupon] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ShowBanner] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CashAble] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SeedAble] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Seed] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SeedGive] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StateDuplicate] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_IsHiddenCashShop] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[UIString_NameID] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtCashCommodity_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtCashCommodity_id] ON [dbo].[ExtCashCommodity]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
