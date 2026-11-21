USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_ItemSales]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_ItemSales](
	[BasisDate] [date] NOT NULL,
	[WorldID] [int] NOT NULL,
	[ItemID] [int] NOT NULL,
	[SaleDirectionCode] [tinyint] NOT NULL,
	[CharacterCount] [int] NOT NULL,
	[SaleCount] [int] NOT NULL,
	[ItemCount] [int] NOT NULL,
 CONSTRAINT [PK_ST_ItemSales] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[WorldID] ASC,
	[ItemID] ASC,
	[SaleDirectionCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, FILLFACTOR = 100) ON [PRIMARY]
) ON [PRIMARY]
GO
