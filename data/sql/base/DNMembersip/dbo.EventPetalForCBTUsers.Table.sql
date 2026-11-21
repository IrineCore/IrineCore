USE [dnmembership]
GO
/****** Object:  Table [dbo].[EventPetalForCBTUsers]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[EventPetalForCBTUsers]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[EventPetalForCBTUsers](
	[AccountName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[PetalAmount] [int] NOT NULL,
	[CompleteFlag] [bit] NOT NULL,
	[PetalIncomeID] [bigint] NULL,
 CONSTRAINT [PK_EventPetalForCBTUsers] PRIMARY KEY CLUSTERED 
(
	[AccountName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
