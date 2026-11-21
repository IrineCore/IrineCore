USE [dnstaging]
GO
/****** Object:  Table [dbo].[SalesAmountRanges]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SalesAmountRanges](
	[SaleAmountRangeCode] [tinyint] NOT NULL,
	[MinAmount] [int] NOT NULL,
	[MaxAmount] [int] NOT NULL,
 CONSTRAINT [PK_SalesAmountRanges] PRIMARY KEY CLUSTERED 
(
	[SaleAmountRangeCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
