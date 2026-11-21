USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailySalesByProduct]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailySalesByProduct](
	[BasisDate] [date] NOT NULL,
	[ProductID] [int] NOT NULL,
	[PayMethodCode] [tinyint] NOT NULL,
	[PetalIncomeCode] [tinyint] NOT NULL,
	[UniqueBuyerCount] [int] NOT NULL,
	[UniqueCharacterCount] [int] NOT NULL,
	[SaleCount] [int] NOT NULL,
	[SaleAmount] [int] NOT NULL,
	[NationalityCode] [tinyint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_DailySalesByProduct_BasisDate_ProductID_PayMethodCode_PetalIncomeCode_NationalityCode]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ST_DailySalesByProduct_BasisDate_ProductID_PayMethodCode_PetalIncomeCode_NationalityCode] ON [dbo].[ST_DailySalesByProduct]
(
	[BasisDate] ASC,
	[ProductID] ASC,
	[PayMethodCode] ASC,
	[PetalIncomeCode] ASC,
	[NationalityCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
