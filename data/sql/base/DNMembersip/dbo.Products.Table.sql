USE [dnmembership]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Products](
	[ProductID] [int] NOT NULL CONSTRAINT [PK_Products] PRIMARY KEY CLUSTERED,
	[Category] [int] NULL,
	[SubCategory] [int] NULL,
	[ItemSort] [int] NULL,
	[ProductName] [nvarchar](200) NULL,
	[ItemID01] [int] NULL,
	[ItemID02] [int] NULL,
	[ItemID03] [int] NULL,
	[ItemID04] [int] NULL,
	[ItemID05] [int] NULL,
	[LinkSN01] [int] NULL,
	[LinkSN02] [int] NULL,
	[LinkSN03] [int] NULL,
	[LinkSN04] [int] NULL,
	[LinkSN05] [int] NULL,
	[Period] [int] NULL,
	[Price] [int] NULL,
	[PriceFix] [int] NULL,
	[Count] [int] NULL,
	[Priority] [int] NULL,
	[OnSale] [bit] NULL,
	[Limit] [bit] NULL,
	[ReserveGive] [bit] NULL,
	[Reserve] [int] NULL,
	[validity] [int] NULL,
	[ResrveAble] [bit] NULL,
	[PresentAble] [bit] NULL,
	[CartAble] [bit] NULL
)
GO
