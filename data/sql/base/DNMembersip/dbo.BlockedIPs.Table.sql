USE [dnmembership]
GO
/****** Object:  Table [dbo].[BlockedIPs]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[BlockedIPs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[BlockedIPs](
	[StartIP] [int] NOT NULL,
	[EndIP] [int] NOT NULL,
	[BlockedIPCode] [tinyint] NOT NULL,
	[Description] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[AdminID] [int] NOT NULL,
	[RegisterDate] [smalldatetime] NOT NULL,
	[StartDate] [smalldatetime] NULL,
	[EndDate] [smalldatetime] NULL,
	[IPBlockMethodCode] [tinyint] NOT NULL,
 CONSTRAINT [PK_BlockedIPs] PRIMARY KEY CLUSTERED 
(
	[StartIP] ASC,
	[EndIP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_BlockedIPs_IPBlockMethodCode]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[BlockedIPs] ADD  CONSTRAINT [DF_BlockedIPs_IPBlockMethodCode]  DEFAULT ((1)) FOR [IPBlockMethodCode]
END
GO
