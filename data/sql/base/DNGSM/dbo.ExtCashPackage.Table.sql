USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtCashPackage]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtCashPackage](
	[id] [int] NULL,
	[_SN] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID00] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID01] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID02] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID03] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID04] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID05] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID06] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID07] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID08] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID09] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID10] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID11] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID12] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID13] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID14] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID15] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID16] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID17] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID18] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CommodityID19] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtCashPackage_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtCashPackage_id] ON [dbo].[ExtCashPackage]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
